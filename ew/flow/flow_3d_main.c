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

#include <glib.h>
#include <utils/utils.h>
#include "flow.h"

//BOP
//
// !ROUTINE: flow - solve the 1d consolidation equation.
//
// !INTERFACE:
//
// hw4 [file]
//
// !INPUT PARAMETERS:
//
// file        if a file name is given the input parameters will be read
//             from that file.  otherwise the user will be propmpted for
//             each of the parameters individually.
//
// !DESCRIPTION:
//
// solve the non-linear 1d unsaturated flow problem in a heterogeneous medium.
// the governing equation is,
//   $$ {\partial \over \partial z} \left( K(\theta) \left( {\partial \psi \over \partial z} + 1 \right) \right) = C(\psi){\partial \psi \over \partial t} \eqno{(1)}$$
// the dirichlet boundary condition at the base is,
// $$ \psi(0,t) = 0 $$
// and the neuman boundary condition at the surface is,
// $$ \psi_z(l,t) = {R \over K(\theta) } - 1 $$
// as equation (1) is non-linear, that is the coefficients, $K$ and $C$ are
// themselves a function of the quantity that we are trying to find, we must
// iterate to that solution for each time step.  we do this by guessing at
// a solution, $\tilde{\psi}$, calculating $K$ and $C$ for this $\tilde{\psi}$,
// and then solving for $\psi$ using these $K$ and $C$.  if $\psi$ and
// $\tilde{\psi}$ compare, then we have found an acceptable solution and can
// move to the next time step.  if the predicted solution does not match the
// calculated solution, then we make a new guess and carry out this precedure
// again.
//
// we predict the solution at the next iteration, $i+1$, as,
// $$ \{ \tilde{\psi} \}_{i+1} = \{ \tilde{\psi} \}_i + \lambda ( \{\psi\}_i - \{ \tilde{\psi} \}_i ) $$
// where $\lambda=.05$.  after we find a solution, we guess at the solution
// for the next time step, $k+1$ as,
// $$ \{ \psi \}^{k+1} = {3\over 2} \{ \psi \}^k - {1\over 2}\{ \psi \}^{k-1} $$
//
// !REVISION HISTORY:
// feb 2002 eric hutton initial version.
//
//EOP
//BOC

#define SED_RATE_DEFAULT (.005)
#define DEPTH_DEFAULT    (1500.)
#define PSI_BASE_DEFAULT (80.)
#define DZ_DEFAULT       (25.)
#define N_DEFAULT        (5)
#define DX_DEFAULT       (25.)
#define DT_DEFAULT       (48000.)
#define END_TIME_DEFAULT (1280000.)
#define PSI_MIN_DEFAULT  (10.)
#define VERBOSE_DEFAULT  (FALSE)

void
print_profile_3d(double t, double*** psi, int n);

static const char* help_msg[] = {
    " flow - sovle the 1d consolidation equation.                         ",
    "                                                                     ",
    " options:                                                            ",
    "   r       : sedimentation rate (m/s)                                ",
    "   d       : initial depth of the sediment (m)                       ",
    "   u0      : excess porewater pressure at the bsae of the model      ",
    "   umin    : excess porewater pressure of surface sediment           ",
    "   dz      : vertical resolution of the model (m)                    ",
    "   dt      : time resolution of the model (s)                        ",
    "   end     : length of the model (s)                                 ",
    "                                                                     ",
    NULL
};

#include <stdlib.h>

int
main(int argc, char* argv[])
{
    double** sed_rate_vec;
    double sed_rate;
    double depth;
    double psi_base;
    double psi_min;
    double dx;
    double dz;
    double dt;
    double dt_init;
    double end_time;
    gboolean verbose;
    int i, j, k, n;
    double t;
    double*** psi;
    double*** kx, *** kz, *** c;
    Eh_args* args;

    args = eh_opts_init(argc, argv);

    if (eh_check_opts(args, NULL, NULL, help_msg) != 0) {
        eh_exit(-1);
    }

    sed_rate = eh_get_opt_dbl(args, "r", SED_RATE_DEFAULT);
    depth    = eh_get_opt_dbl(args, "d", DEPTH_DEFAULT);
    psi_base = eh_get_opt_dbl(args, "u0", PSI_BASE_DEFAULT);
    psi_min  = eh_get_opt_dbl(args, "umin", PSI_MIN_DEFAULT);
    dz       = eh_get_opt_dbl(args, "dz", DZ_DEFAULT);
    n        = eh_get_opt_int(args, "n", N_DEFAULT);
    dx       = eh_get_opt_dbl(args, "dx", DZ_DEFAULT);
    dt_init  = eh_get_opt_dbl(args, "dt", DT_DEFAULT);
    end_time = eh_get_opt_dbl(args, "end", END_TIME_DEFAULT);
    verbose  = eh_get_opt_bool(args, "v", VERBOSE_DEFAULT);

    if (verbose) {
        eh_print_opt(args, "r");
        eh_print_opt(args, "d");
        eh_print_opt(args, "u0");
        eh_print_opt(args, "umin");
        eh_print_opt(args, "dz");
        eh_print_opt(args, "dt");
        eh_print_opt(args, "end");
        eh_print_opt(args, "n");
    }

    // the number of nodes.
    n = pow(2, n) + 1;

    dz = depth / (n - 1);

    // allocate memory.
    //   psi = allocate_3d( n );
    psi = eh_new(double**, n);

    for (i = 0 ; i < n ; i++) {
        psi[i] = eh_new(double*, n);
        //      for ( j=0 ; j<n ; j++ )
        //         psi[i][j] = eh_new( double , n );
    }

    kx  = allocate_3d(n);
    kz  = allocate_3d(n);
    c   = allocate_3d(n);

    for (i = 0 ; i < n ; i++) {
        for (j = 0 ; j < n ; j++) {
            psi[i][j] = eh_linspace(psi_base, psi_min, n);

            for (k = 0 ; k < n ; k++) {
                psi[i][j][k] = 1.;
                kx[i][j][k]  = 1;
                kz[i][j][k]  = 1;
                c[i][j][k]   = 1.;
            }
        }
    }

    for (i = 0 ; i < n ; i++) {
        for (j = 0 ; j < n ; j++)
            for (k = 0 ; k < n ; k++) {
                /*
                            if ( i>n/4 && j>n/4  && (k>n/4 && k<n/2)  )
                            {
                               kx[i][j][k] /= 100;
                               kz[i][j][k] /= 1000;
                            }
                */
                /*
                            if ( (k>n/2 && k<3*n/4)  )
                            {
                               kx[i][j][k] /= 50;
                               kz[i][j][k] /= 1000;
                            }
                */
            }

    }

    sed_rate_vec = allocate_2d(n);

    for (i = 0 ; i < n ; i++)
        for (j = 0 ; j < n ; j++) {
            psi[i][j][n - 1]     = 0;
            sed_rate_vec[i][j] = 0;

            /*
                     if ( (i>=(n-1)/2-1 && i<=(n-1)/2+1) && (j>=(n-1)/2-1 && j<=(n-1)/2+1) )
                        sed_rate_vec[i][j] = sed_rate*10000;
            */

        }

    //   sed_rate_vec[(n-1)/2][(n-1)/2] = sed_rate*dz*dz;

    // write out the initial conditions.
    print_profile_3d(t, psi, n);

    for (t = 0 ; t < end_time ; t += dt) {

        /*
              if ( t>end_time/2 )
                 for ( i=0 ; i<n ; i++ )
                    for ( j=0 ; j<n ; j++ )
                       sed_rate_vec[i][j] = 0;
        */

        dt = dt_init;

        solve_excess_pore_pressure_mg_3d(psi, kx, kz, c, n, dx, dz, dt, sed_rate_vec);

        // write out the solution for this time step.
        print_profile_3d(t + dt, psi, n);

    }

    return 0;
}
//EOC

void
print_profile_3d(double t, double*** psi, int n)
{
    int i, j, k;

    for (k = 0 ; k < n ; k++)
        for (j = 0 ; j < n ; j++)
            for (i = 0 ; i < n ; i++) {
                fwrite(&(psi[i][j][k]), sizeof(double), 1, stdout);
            }

    //   fwrite( psi[0][0] , sizeof(double) , n*n*n , stdout );
    fflush(stdout);
}

