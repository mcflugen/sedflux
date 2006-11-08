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

static char *help_msg[] = {
" ad_dif - solve the advection diffusion equation.                    ",
"                                                                     ",
" options:                                                            ",
"   k    : diffusion coefficient.                                     ",
"   ws   : sediment settling velocity.                                ",
"   c0   : concentration gradient at the start of the model domain.   ",
"   cn   : concentration at the end of the model domain.              ",
"   x0   : location of the begining of the model domain.              ",
"   xn   : location of end of model domain.                           ",
"   tend : length of time to run the model for                        ",
"   dt   : model time step                                            ",
"   n    : number of nodes for the model grid                         ",
"                                                                     ",
NULL };

#define K_DEFAULT       (.5)
#define WS_DEFAULT      (.5)
#define C0_DEFAULT      (-.1)
#define CN_DEFAULT      (0.)
#define X0_DEFAULT      (.01)
#define XN_DEFAULT      (100)
#define ALPHA_DEFAULT   (1)
#define TEND_DEFAULT    (100.)
#define DT_DEFAULT      (1.)
#define VERBOSE_DEFAULT (FALSE)
#define N_DEFAULT       (256)

int main( int argc , char *argv[] )
{
   char *possible[] = {"k","ws","c0","cn","x0","xn","dt","tend","a","v","n",NULL};
   double k, w_s, c_0, c_n, x_0, x_n;
   double dt, t_end;
   double alpha;
   gboolean verbose;
   double *node, *c;
   Advec_element *e;
   Advec_const a_const;
   int i;
   int n_nodes, n_elements;
   Eh_args *args;

   args = eh_opts_init( argc , argv );
   if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
      eh_exit(-1);

   k       = eh_get_opt_dbl ( args , "k"    , K_DEFAULT       );
   w_s     = eh_get_opt_dbl ( args , "ws"   , WS_DEFAULT      );
   c_0     = eh_get_opt_dbl ( args , "c0"   , C0_DEFAULT      );
   c_n     = eh_get_opt_dbl ( args , "cn"   , CN_DEFAULT      );
   x_0     = eh_get_opt_dbl ( args , "x0"   , X0_DEFAULT      );
   x_n     = eh_get_opt_dbl ( args , "xn"   , XN_DEFAULT      );
   dt      = eh_get_opt_dbl ( args , "dt"   , DT_DEFAULT      );
   t_end   = eh_get_opt_dbl ( args , "tend" , TEND_DEFAULT    );
   alpha   = eh_get_opt_dbl ( args , "a"    , ALPHA_DEFAULT   );
   n_nodes = eh_get_opt_int ( args , "n"    , N_DEFAULT       );
   verbose = eh_get_opt_bool( args , "v"    , VERBOSE_DEFAULT );

   if ( verbose )
   {
      eh_print_opt( args , "k"    );
      eh_print_opt( args , "ws"   );
      eh_print_opt( args , "c0"   );
      eh_print_opt( args , "cn"   );
      eh_print_opt( args , "x0"   );
      eh_print_opt( args , "xn"   );
      eh_print_opt( args , "dt"   );
      eh_print_opt( args , "tend" );
      eh_print_opt( args , "a"    );
      eh_print_opt( args , "n"    );
   }

   // the number of elements will be one less than the number of nodes.
   n_elements = n_nodes-1;

   // allocate memory.
   c    = g_new0( double        , n_nodes    );
   e    = g_new ( Advec_element , n_elements );

   // initialize the positions of each of the nodes.
   node = eh_logspace( log10(x_0) , log10(x_n) , n_nodes );
//   node = eh_linspace( x_0 , x_n , n_nodes );

   // define the element parameters.  here k, and w_s are constant but
   // they don't need to be.
   for ( i=0 ; i<n_elements ; i++ )
   {
      e[i].x_1 = node[i];
      e[i].x_2 = node[i+1];
      e[i].k   = k;
      e[i].v   = w_s;
   }

   // initialize the initial concentration profile.
   for ( i=0 ; i<n_nodes ; i++ )
      c[i] = 0;

   for ( i=n_nodes/2 ; i<3*n_nodes/4 ; i++ )
      c[i] = .5;

   a_const.dt = dt;
   a_const.t_end = t_end;
   a_const.alpha = alpha;

   a_const.bc_lower_type = BC_TYPE_NEUMAN;
   a_const.bc_upper_type = BC_TYPE_DIRICHLET;
   a_const.bc_lower_val  = c_0;
   a_const.bc_upper_val  = c_n;

   // print the node positions.
   print_output( 0. , node , n_nodes );

   advec( e , c , n_nodes , a_const );

   free( c );
   free( e );

   return 0;
}

