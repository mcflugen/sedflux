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

/*
 * PlumeRead.c  Reads the input file for Plume
 *
 *
 *  Author:     M.D. Morehead
 *  Original:   April 1998
 *  Modified:   Sep 1998, MDM, Conversion for SEDFLUX3D
 *
 */
#include "plumeinput.h"
#include "plumevars.h"
#include <stdio.h>

int
plumeread(FILE* fpin, Plume_enviro* env, Plume_grid* grid, Plume_options* opt)
{
    char chs[120];
    int  ii;
    double lambdad; // temporary lambda (1/day)
    Plume_river* river = env->river;
    Plume_ocean* ocean = env->ocean;

    /*
     *   read in the River parameters
     *   lines 1-4
     */
    fscanf(fpin, "%lf", &river->Q);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%lf", &river->u0);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%lf", &river->rdirection);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%lf", &river->b0);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%lf", &river->rma);
    fgets(chs, 120, fpin);
    /*
     *   Sediment Load Parameters
     *   line 5
     *   lines 6-8 (multiple sets)
     */
    fscanf(fpin, "%2d", &env->n_grains);
    fgets(chs, 120, fpin);

    river->Cs = eh_new(double, env->n_grains);

    env->sed = eh_new(Plume_sediment, env->n_grains);

    for (ii = 0; ii < env->n_grains; ii++) {
        fscanf(fpin, "%lf", &river->Cs[ii]);
        fgets(chs, 120, fpin);
        fscanf(fpin, "%lf", &lambdad);
        fgets(chs, 120, fpin);
        fscanf(fpin, "%lf", &env->sed[ii].rho);
        fgets(chs, 120, fpin);
        env->sed[ii].lambda = lambdad / dTOs;     /* lambda (1/s) */
    } /* end for */

    /*
     *   Ocean Parameters
     *   lines 9-13
     */
    fscanf(fpin, "%lf", &ocean->Cw);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%lf", &ocean->vo);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%lf", &ocean->vdirection);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%lf", &ocean->cc);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%lf", &ocean->So);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%lf", &ocean->Sw);
    fgets(chs, 120, fpin);
    /*
     *   Other Parameters
     *   line 14-16
     */
    fscanf(fpin, "%1d", &opt->fjrd);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%1d", &opt->kwf);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%d", &env->lat);
    fgets(chs, 120, fpin);

    /*
     *   grids
     *   lines 17-20
     */
    fscanf(fpin, "%2d", &grid->ndy);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%2d", &grid->ndx);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%lf", &grid->ymin);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%lf", &grid->ymax);
    fgets(chs, 120, fpin);
    grid->x_len = 0;
    grid->y_len = 0;

    /*
     *   debug and output option flags
     *   line 21, dbg removed
     *   lines 22-24
     */
    fscanf(fpin, "%d", &opt->o1);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%d", &opt->o2);
    fgets(chs, 120, fpin);
    fscanf(fpin, "%d", &opt->o3);

    fclose(fpin);
    return (0);
} // end of PlumeRead

