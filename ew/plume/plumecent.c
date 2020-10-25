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
 * PlumeCent    Calculates the position; distance; and velocity along
 *          the deformed centerline for PLUME
 *
 *  Author:     M.D. Morehead
 *  Original:   April 1998
 *
 *
 *  Define:
 *      Li   = Inertial distance, u/f
 *      Licc = Inertial distance of the coastal current
 *      ccw  = coastal current width
 *      pra  = plume rotation angle
 *
 */
#include "plumeinput.h"
#include "plumevars.h"
#include <utils/utils.h>

/*
 *  Start of PlumeCent
 */
int
plumecent(Plume_enviro* env, Plume_grid* grid, Plume_options* opt)
{
#ifdef DBG
    FILE* fid1;  /* dbg file */
#endif
    int ii, jj, cci, ll, lp, ll1, ll2, ll3, ll4, zclind, zmi;
    double Li, Licc, ccw, ccw2, dccw, ucc;
    double aa, AA, avo, nv, v1, v2, zm;
    double dyz[4], sin1, cos1, tst, dcl, pra;
    double** pct0, **pct1, **pct2, **pct3;
    Plume_river river = *(env->river);
    Plume_ocean ocean = *(env->ocean);

    // Straight Centerline
    if (opt->fjrd || opt->strt) {
        // vo ~ 0
        for (ii = 0 ; ii < grid->lx ; ii++) {
            grid->pcent[ii][0] = grid->xval[ii];
            grid->pcent[ii][1] = 0;
            grid->pcent[ii][2] = grid->xval[ii];
        }

        grid->lc = grid->lx;

        // Calculate Centerline velocity from Albertsons Equations
        for (ii = 0 ; ii < grid->lc ; ii++) {
            if (grid->pcent[ii][2] < plg * river.b0) { // zone of flow establishment
                grid->pcent[ii][3] = river.u0;
            } else {                            // zone of established flow
                v1 = river.b0 / (sqpi * C1 * grid->pcent[ii][2]); // bo/(sqrt(pi)*C1*x)
                v2 = 0;                               // y/(sqrt(2)*C1*x)
                grid->pcent[ii][3] = river.u0 * sqrt(v1) * exp(sq(-v2));
            }
        }
    } else {
        //---
        // Deform the centerline velocity with the alongshore current
        //
        // Deflected Jet centerline constants
        //---
        avo = fabs(ocean.vo);
        aa  = river.u0 / avo;     // a = Wmo/Ue
        nv  = 0.37;           // n = [ 0.375 0.380 0.370 0.385 ]
        AA  = 1.53 + 0.90 * aa;

        //---
        // Determine subsample interval to barely guarentee the first x value
        //  take the equation: z1 = bo*AA*(y1(jj)/bo)^nv,
        //  invert it, set z1 = dx, find y1, and set dyz to this ...
        //  use mutliple values to reduce the total array size for small vo
        //---
        dyz[0] = 0.9 * mn(river.b0 * pow(grid->dx / (river.b0 * AA), 1 / nv),
                grid->dy);
        dyz[1] = 0.9 * mn(river.b0 * pow((100. + grid->dx) / (river.b0 * AA), 1 / nv)
                - river.b0 * pow(100 / (river.b0 * AA), 1 / nv),
                grid->dy);
        dyz[2] = 0.9 * mn(river.b0 * pow((1000. + grid->dx) / (river.b0 * AA), 1 / nv)
                - river.b0 * pow(1000 / (river.b0 * AA), 1 / nv),
                grid->dy);
        dyz[3] = 0.9 * mn(river.b0 * pow((10000. + grid->dx) / (river.b0 * AA), 1 / nv)
                - river.b0 * pow(10000 / (river.b0 * AA), 1 / nv),
                grid->dy);

        // Initial Grid
        ll1 = ((int)((100 -   0) / dyz[0])) + 1;
        ll2 = ((int)((1000 - 100) / dyz[1])) + 1;
        ll3 = ((int)((10000 - 1000) / dyz[2])) + 1;
        ll4 = ((int)((mx(1.3 * grid->ymax, -1.3 * grid->ymin) - 10000) / dyz[3])) + 2;
        ll = ll1 + ll2 + ll3 + ll4;

#ifdef DBG
        fprintf(stderr, "  PlumeCent: Allocating pct matrices \n");
#endif
        pct0 = new_dmatrix(ll, 2);
        pct1 = new_dmatrix(ll, 2);
        pct2 = new_dmatrix(ll, 2);
        pct3 = new_dmatrix(grid->lpc, 2);
#ifdef DBG
        fprintf(stderr, "          finished Allocation \n");
#endif

        pct0[0][1] = 0;

        for (ii = 1 ; ii < ll ; ii++) {
            if (pct0[ii - 1][1] < 100) {
                pct0[ii][1] = pct0[ii - 1][1] + dyz[0];
            } else if (pct0[ii - 1][1] < 1000) {
                pct0[ii][1] = pct0[ii - 1][1] + dyz[1];
            } else if (pct0[ii - 1][1] < 10000) {
                pct0[ii][1] = pct0[ii - 1][1] + dyz[2];
            } else {
                pct0[ii][1] = pct0[ii - 1][1] + dyz[3];
            }
        }

        //---
        // First, Solve for the position in the + y direction
        //  later will map onto the xvals,yvals grid using pcent
        //  in the appropriate direction
        //
        // Find the jet position unaffected by Coriolis and Buoyancy
        //---
        for (ii = 0 ; ii < ll ; ii++) {
            pct0[ii][0] = river.b0 * AA * pow(pct0[ii][1] / river.b0, nv);
        }

        // Rotate by the river mouth angle (rma)
        sin1 = sin(river.rma * degTOr);
        cos1 = cos(river.rma * degTOr);

        for (ii = 0 ; ii < ll ; ii++) {
            pct1[ii][0] = -pct0[ii][1] * sin1 + pct0[ii][0] * cos1;
            pct1[ii][1] =  pct0[ii][1] * cos1 + pct0[ii][0] * sin1;
        }

        // if in the direction of Kelvin wave propagation (kwf == 1)
        if (opt->kwf) {

            // Find the Length of an inertial circle (Li: r = V/f ),
            //  calculate Li based on (uo+vo)/2
            Li = 0.5 * (river.u0 + avo) / (2 * omega * sin(env->lat * degTOr));

            // calculate coastal current width (ccw) based on the
            //  plume centerline velocity at the distance 10*Li
            ucc    = river.u0 * sqrt(river.b0 / (sqpi * C1 * 10 * Li));
            Licc   = ucc / (2 * omega * sin(env->lat * degTOr));
            ccw    = ocean.cc * Licc;

            // find the tangential distance to the start of coastal current
            //  assumes the coastal current starts at 2*Li downstream
            dccw   = sqrt(pow(2 * Li, 2) + pow(ccw, 2));

            // find the same distance to the unmodified centerline pts
            //  numerically minimize the quadratic equation z^2 - x^2 - y^2 = 0
            zclind = 0;
            tst = fabs(pow(dccw, 2) - pow(pct1[0][0], 2) - pow(pct1[0][1], 2));

            for (ii = 0 ; ii < ll ; ii++) {
                if (fabs(pow(dccw, 2) - pow(pct1[ii][0], 2) - pow(pct1[ii][1], 2))
                    < tst) {
                    zclind = ii;
                    tst = fabs(pow(dccw, 2)
                            - pow(pct1[ii][0], 2)
                            - pow(pct1[ii][1], 2));
                }
            }

            dcl    = sqrt(pow(pct1[zclind][0], 2) + pow(pct1[zclind][1], 2));

            //---
            // find the plume rotation angle to match the jet to the coastal
            //  current
            // pra = (angle to jet endpoint) - (angle to cc endpoint)
            //---
            pra    = rTOdeg * (atan2(pct1[zclind][0], pct1[zclind][1]) - atan2(ccw, 2 * Li));

            //---
            // if the rotation angle is greater than the river mouth angle
            //   rotate the plume so the centerline meets the coastal current
            //   otherwise leave at rma
            //---
            if (pra > 0) {
                sin1 = sin(pra * degTOr);
                cos1 = cos(pra * degTOr);
            } else {
                sin1 = 0;
                cos1 = 1;
            }

            for (ii = 0 ; ii < ll ; ii++) {
                pct2[ii][0] = -pct1[ii][1] * sin1 + pct1[ii][0] * cos1;
                pct2[ii][1] =  pct1[ii][1] * cos1 + pct1[ii][0] * sin1;
            }
        } else {
            //---
            // if not in the direction of Kelvin wave propagation (already
            //  rotated by rma)
            //---
            for (ii = 0 ; ii < ll ; ii++) {
                pct2[ii][0] = pct1[ii][0];
                pct2[ii][1] = pct1[ii][1];
            }
        }

        //---
        // Interpolate the subsampled points onto the standard grid spacing
        //  only keeping unique points, and find the maximum
        //---
        pct3[0][0] = rnd(pct2[0][0] / grid->dx) * grid->dx;
        pct3[0][1] = rnd(pct2[0][1] / grid->dy) * grid->dy;
        zmi = 0;
        zm = pct3[0][0];
        lp = 1;

        for (ii = 1 ; ii < ll ; ii++) {
            v1 = rnd(pct2[ii][0] / grid->dx) * grid->dx;
            v2 = rnd(pct2[ii][1] / grid->dy) * grid->dy;

            if (v1 != pct3[lp - 1][0] || v2 != pct3[lp - 1][1]) {
                pct3[lp][0] = v1;
                pct3[lp][1] = v2;

                if (v1 > zm) {
                    zm  = v1;
                    zmi = lp;
                }

                lp++;

                if (lp > grid->lpc) {
                    fprintf(stderr, "PlumeCENT Error (a): pct array is too small. \n");
                    fprintf(stderr, "   lp > lpc \n");
                    fprintf(stderr, "   increase lpc in PlumeArray.c\n");
                    fprintf(stderr, "   lpc = %d \n", grid->lpc);
                    fprintf(stderr, "   lp  > %d \n", lp);
                    fprintf(stderr, "   ii  = %d \n", ii);
                    fprintf(stderr, "   ll  = %d \n", ll);
                    return 1;
                }
            }
        }

        //---
        // Adjust for location of coastal current (y > 2*Li)
        //---
        if (opt->kwf != 0) {
            // find the nearest grid point to the cc width
            ccw2 = rnd(ccw / grid->dx) * grid->dx;

            // if local max in centerline and returns to ccw
            if (zm > ccw2 && pct3[zmi][1] <= 2 * Li) {
                ii = zmi;

                while (pct3[ii][0] > ccw2) {
                    ii++;
                }

                cci = ii;
            } else if (zm > ccw2) { // ever increasing function but crosses centerline
                ii = 0;

                while (pct3[ii][0] < ccw2) {
                    ii++;
                }

                cci = ii;
            } else {
                fprintf(stderr, "\n\n PlumeCent ERROR: program aborted \n");
                fprintf(stderr, "    Plume was over-rotated and never reached ccw \n");
                return 1;
            }

            // set all centerline values greater than this to ccw
            ii = cci;

            while (ii < lp) {
                pct3[ii][0] = ccw2;
                ii++;
            }
        }

        //---
        // place into the pcent arrays
        //    [lx+ly,4]  -    deflected centerline information
        //    [:,0:1]    m    centerline position (x,y)
        //    [:,2]      m    distance from the river mouth, along the centerline
        //    [:,3]      m/s  Albertson Velocity at the centerline point
        //                     lc is actual end point
        //---
        if (ocean.vo > 0 && opt->kwf == 0) {   // may exit top or side
            if (pct3[lp - 1][0] > grid->xmax) { // check for exiting top
                ii = lp - 1;

                while (pct3[ii][0] > grid->xmax) {
                    ii--;
                }
            } else {
                ii = lp - 1;
            }

            if (pct3[lp - 1][1] > grid->ymax) { // check for exiting side
                jj = lp - 1;

                while (pct3[jj][1] > grid->ymax) {
                    jj--;
                }
            } else {
                jj = lp - 1;
            }

            grid->lc = mn(ii, jj) + 1;

            for (ii = 0 ; ii < grid->lc ; ii++) {
                grid->pcent[ii][0] = pct3[ii][0];
                grid->pcent[ii][1] = pct3[ii][1];
            }
        } else if (ocean.vo > 0 && opt->kwf != 0) { // assume it will not exit the top
            if (pct3[lp - 1][1] < grid->ymax) {
                fprintf(stderr, "PlumeCent Error (b): not enough centerline grid points, \n");
                fprintf(stderr, "    the centerline does not exit the array.\n");
                fprintf(stderr, "    pct3[lp-1][1] < ymax \n");
                fprintf(stderr, "    Increase lpc in PlumeArray.c \n");
                fprintf(stderr, "    lpc = %d \n", grid->lpc);
                fprintf(stderr, "    lp  = %d \n", lp);
                fprintf(stderr, "    ymax  = %f \n", grid->ymax);
                fprintf(stderr, "    pct3[lp-1][1] = %g \n", pct3[lp - 1][1]);
                return 1;
            }

            jj = lp - 1;

            while (pct3[jj][1] > grid->ymax) {
                jj--;
            }

            grid->lc = jj + 1;

            for (ii = 0 ; ii < grid->lc ; ii++) {
                grid->pcent[ii][0] = pct3[ii][0];
                grid->pcent[ii][1] = pct3[ii][1];
            }
        } else if (ocean.vo < 0 && opt->kwf == 0) { // may exit top or side
            if (pct3[lp - 1][0] > grid->xmax) { // check for exiting top
                ii = lp - 1;

                while (pct3[ii][0] > grid->xmax) {
                    ii--;
                }
            } else {
                ii = lp - 1;
            }

            if (-pct3[lp - 1][1] < grid->ymin) { // check for exiting side
                jj = lp - 1;

                while (-pct3[jj][1] < grid->ymin) {
                    jj--;
                }
            } else {
                jj = lp - 1;
            }

            grid->lc = mn(ii, jj) + 1;

            for (ii = 0 ; ii < grid->lc ; ii++) {
                grid->pcent[ii][0] =  pct3[ii][0];
                grid->pcent[ii][1] = -pct3[ii][1];
            }
        } else { // assume it will not exit the top
            if (-pct3[lp - 1][1] > grid->ymin) {
                fprintf(stderr, "PlumeCent Error (c): not enough centerline grid points, \n");
                fprintf(stderr, "    the centerline does not exit the array.\n");
                fprintf(stderr, "    -pct3[lp-1][1] > ymin \n");
                fprintf(stderr, "    Increase lpc in PlumeArray.c \n");
                fprintf(stderr, "    lpc = %d \n", grid->lpc);
                fprintf(stderr, "    lp  = %d \n", lp);
                fprintf(stderr, "    ymin  = %f \n", grid->ymin);
                fprintf(stderr, "    -pct3[lp-1][1]  = %g \n", -pct3[lp - 1][1]);
                return 1;
            }

            jj = lp - 1;

            while (-pct3[jj][1] < grid->ymin) {
                jj--;
            }

            grid->lc = jj + 1;

            for (ii = 0 ; ii < grid->lc ; ii++) {
                grid->pcent[ii][0] =  pct3[ii][0];
                grid->pcent[ii][1] = -pct3[ii][1];
            }
        } // end if() filling pcent[:][1:2] arrays

        //---
        // find the distance along the centerline and the velocity
        //---
        grid->pcent[0][2] = grid->xval[0];   // distance along centerline
        grid->pcent[0][3] = river.u0;  // Albertson Velocity

        for (ii = 1 ; ii < grid->lc ; ii++) {
            //---
            // distance along the plume
            // (last distance + distance to last point)
            //---
            grid->pcent[ii][2] = grid->pcent[ii - 1][2]
                + sqrt(sq(grid->pcent[ii][0] - grid->pcent[ii - 1][0])
                    + sq(grid->pcent[ii][1] - grid->pcent[ii - 1][1]));

            // centerline velocity along the plume
            if (grid->pcent[ii][2] < plg * river.b0) { // 'zone of flow establishment'
                grid->pcent[ii][3] = river.u0;
            } else {                           // 'zone of established flow'
                v1 = river.b0 / (sqpi * C1 * grid->pcent[ii][2]); // bo/(sqrt(pi)*C1*x)
                v2 = 0;                               // y/(sqrt(2)*C1*x)
                grid->pcent[ii][3] = river.u0 * sqrt(v1) * exp(-sq(v2));
            }
        }

        // Free up allocated memory
        free_dmatrix(pct0);
        free_dmatrix(pct1);
        free_dmatrix(pct2);
        free_dmatrix(pct3);

    } // end ifelse( fjrd || strt )

#ifdef DBG
    fid1 = fopen("pcnt.dat", "w");

    for (ii = 0 ; ii < grid->lc ; ii++) {
        fprintf(fid1, "%g %g %g %g \n", grid->pcent[ii][0], grid->pcent[ii][1],
            grid->pcent[ii][2], grid->pcent[ii][3]);
    }

    fclose(fid1);
#endif

    return 0;

} // end of PlumeCent

