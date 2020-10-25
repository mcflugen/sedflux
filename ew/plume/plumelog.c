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
 * PlumeLog.c   Prints various informative paramters to a log file for Plume
 *
 *  Author:     M.D. Morehead
 *  Original:   May 1998
 *
 */
#include "plumeinput.h"
#include "plumevars.h"
#include <stdio.h>
#include <time.h>

int
plumelog(Plume_enviro* env, Plume_grid* grid, Plume_options* opt, Plume_mass_bal* mb)
{
    time_t   tloc;
    int  nn;
    Plume_river river = *(env->river);
    Plume_ocean ocean = *(env->ocean);
    Plume_sediment* sedload = env->sed;

    // Temporary printout to insure proper values
#ifdef DBG
    fprintf(fidlog, " --- PlumeVars.H --- \n\n");
    fprintf(fidlog, " Constants:\n\n");
    fprintf(fidlog, " Seconds/day \t = %g \t dTOs \t (s/day) \n", dTOs);
    fprintf(fidlog, " g \t\t = %g \t grv \t (m/s^2) \n", grv);
    fprintf(fidlog, " Degree/radian \t = %g \t rTOdeg\t (-) \n", rTOdeg);
    fprintf(fidlog, " Radian/degree \t = %g \t degTOr\t (-) \n", degTOr);
    fprintf(fidlog, " sqrt(pi) \t = %g \t sqpi \t (-) \n", sqpi);
    fprintf(fidlog, " sqrt(2) \t = %g \t sqtwo \t (-) \n", sqtwo);
    fprintf(fidlog, " Earths rot. \t = %g \t omega \t (s) \n\n", omega);
#endif

    // Model Coefficients
    fprintf(fidlog, " Model Coefficients:\n\n");
    fprintf(fidlog, " Albertson Constant \t = %g \t C1 \t (-) \n", C1);
    fprintf(fidlog, " Critical Velocity \t = %g \t ucrit \t (m/s) \n", ucrit);
    fprintf(fidlog, " Critical Conc. \t = %g \t pcrit \t (kg/m^3) \n", pcrit);
    fprintf(fidlog, " Length of Plug Flow \t = %g \t plg \t (river widths) \n", plg);
    fprintf(fidlog, " Plume spreading angle \t = %g \t\t sprd \t (deg) \n",
        rTOdeg * atan(sprd));
    fprintf(fidlog, " #Pts searched in PlumeCent = %d \t\t npts \t (-) \n", npts);
    fprintf(fidlog, " Max. Mass Balance Error = %g \t\t mberr \t (%%) \n\n", mberr);

    fprintf(fidlog, " --- Plume Inputs --- \n\n");
    fprintf(fidlog, " River:\t Discharge \t = %g \t river.Q \t (m^3/s) \n", river.Q);
    fprintf(fidlog, " \t Velocity \t = %g \t\t river.u0 \t (m/s) \n", river.u0);
    fprintf(fidlog, " \t Width \t\t = %g \t river.b0 \t (m) \n", river.b0);
    fprintf(fidlog, " \t Depth \t\t = %g \t river.d0 \t (m) \n", river.d0);
    fprintf(fidlog, " \t Angle \t\t = %g \t\t river.rma \t (deg) \n\n", river.rma);

    // Sediment Load Parameters
    fprintf(fidlog, " Sediment Load: ngrains = %d \n", env->n_grains);
    fprintf(fidlog, " \t Grain# \t Lambda (1/day)\t Rho (kg/m^3)\t Cs (kg/m^3) \n");

    for (nn = 0 ; nn < env->n_grains ; nn++) {
        fprintf(fidlog, " \t %d \t\t %g \t\t %g \t %g \n",
            nn, sedload[nn].lambda * dTOs, sedload[nn].rho, river.Cs[nn]);
    }

    // Ocean Parameters
    fprintf(fidlog, "\n Ocean:\t Conc. \t\t\t = %g \t ocean.Cw \t (kg/m^3) \n", ocean.Cw);
    fprintf(fidlog, " \t Valong \t\t = %g \t\t ocean.vo \t (m/s) \n", ocean.vo);
    fprintf(fidlog, " \t River Tracer Conc. \t = %g \t\t ocean.So \t (-) \n", ocean.So);
    fprintf(fidlog, " \t Ocean Tracer Conc. \t = %g \t ocean.Sw \t (-) \n", ocean.Sw);
    fprintf(fidlog, " \t Coast Current Width \t = %g \t\t ocean.cc \t (%% Li) \n\n",
        ocean.cc);

    // Other Parameters
    fprintf(fidlog, " Other Parameters:\n\n");
    fprintf(fidlog, " Straight Centerline Flag \t = %d \t\t strt \t (-) \n", opt->strt);
    fprintf(fidlog, " Fjord Flag \t\t\t = %d \t\t fjrd \t (-) \n", opt->fjrd);
    fprintf(fidlog, " Kelvin Wave Flag \t\t = %d \t\t kwf \t (-) \n", opt->kwf);
    fprintf(fidlog, " Latitude \t\t\t = %d \t lat \t (deg. N) \n\n", env->lat);

    // grids
    fprintf(fidlog, " --- Spatial Array --- \n\n");
    fprintf(fidlog, " Points in the River mouth \t = %d \t ndy \n", grid->ndy);
    fprintf(fidlog, " X points per Y points \t\t = %d \t ndx \n\n", grid->ndx);

    fprintf(fidlog, " Ygrid: # of Y points \t = %d \t\t ly \t (-) \n", grid->ly);
    fprintf(fidlog, " \t grid spacing \t = %f \t\t dy \t (m) \n", grid->dy);
    fprintf(fidlog, " \t Minimum \t = %g \t ymin \t (m) \n", grid->ymin);
    fprintf(fidlog, " \t Maximum \t = %g \t ymax \t (m) \n", grid->ymax);
    fprintf(fidlog, " \t Y=0 index \t = %d \t\t zy \t (-) \n", grid->zy);
    fprintf(fidlog, " \t yval[zy] \t = %g \t\t\t (m) \n\n", grid->yval[grid->zy]);

    fprintf(fidlog, " Xgrid: # of X points \t = %d \t\t lx \t (-) \n", grid->lx);
    fprintf(fidlog, " \t grid spacing \t = %f \t\t dx \t (m) \n", grid->dx);
    fprintf(fidlog, " \t Minimum \t = %g \t\t xmin \t (m) \n", grid->xmin);
    fprintf(fidlog, " \t Maximum \t = %g \t xmax \t (m) \n", grid->xmax);
    fprintf(fidlog, " \t X=0 index \t = %d \t\t zx \t (-) \n", grid->zx);
    fprintf(fidlog, " \t xval[zx] \t = %g \t\t\t (m) \n\n", grid->xval[grid->zx]);

    // Mass Balance Values
    fprintf(fidlog, " --- Mass Balance Summary --- \n\n");
    fprintf(fidlog, "Sediment Balance (kg/day):\n");
    fprintf(fidlog, "\t Qs discharging from river \t = %g \t Tsr \n", mb->Tsr);
    fprintf(fidlog, "\t Initial seafloor Deposit \t = %g \t Tsd[0] \n", mb->Tsd[0]);
    fprintf(fidlog, "\t Percent error \t\t\t = %g \t merr \n", mb->merr);
    fprintf(fidlog, "\t Modified seafloor Deposit\t = %g \t Tsd[1] \n\n", mb->Tsd[1]);

    if (opt->strt) {
        fprintf(fidlog, "Sediment Flux Balance (kg/s):\n");
        fprintf(fidlog, "\t Qs (river) = %g \t Qsr      \n", mb->Qsr);
        fprintf(fidlog, "\t Qs ( 5 km) = %g \t Qsw[0]   \n", mb->Qsw[0]);
        fprintf(fidlog, "\t Qs (10 km) = %g \t Qsw[1]   \n", mb->Qsw[1]);
        fprintf(fidlog, "\t Qs (20 km) = %g \t Qsw[2]   \n", mb->Qsw[2]);
        fprintf(fidlog, "\t Qs (40 km) = %g \t Qsw[3] \n\n", mb->Qsw[3]);
    }

    // Print the program stop time
    tloc = time(&tloc);

    /*
    localtime_r( &tloc, &timeptr );
    strftime( pst, TMLEN, "%X  %x", &timeptr );
    fprintf(fidlog,"\n Stop: %19s \n", pst);
    */

    fprintf(fidlog, "\n Stop: %s\n", ctime(&tloc));
    fprintf(fidlog, " ------------------------- \n\n");

    return (0);

} // end of PlumeLog

