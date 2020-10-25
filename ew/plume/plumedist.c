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
 * PlumeDist    Calculate distance along/from centerline to all
 *          other points in the PLUME array "dist"
 *
 *  Author:     M.D. Morehead
 *  Original:   May 1998
 *
 * dist     lx,ly,5     -   information relating to closest centerline pt.
 *      [:,:,0:1]   -   x,y indices of the closest centerline point
 *          [:,:,2:3]   m   distances along/from centerline
 *      [:,:,4]     m/s Albertson velocity at the centerline point
 *
 */
#include "plumeinput.h"
#include "plumevars.h"
#include <utils/utils.h>

int
plumedist(Plume_grid* grid, Plume_options* opt)
{
    void itoa(int, char[]);
#ifdef DBG
    FILE* fiddbg;            // dbg file
    char fndbg[11], ss[3];
#endif
    int ii, jj, kk, k1, k2, kin;
    double dst1, dst0, rdis;

    // Max distance within the grid, for initial test value
    dst0 = sqrt(sq((grid->ymax - grid->ymin)) + sq((grid->xmax - grid->xmin)));

    if (opt->fjrd || opt->strt) { // vo ~ 0

        for (ii = 0 ; ii < grid->lx ; ii++) {
            for (jj = 0 ; jj < grid->ly ; jj++) {
                grid->dist[ii][jj][0] = (double)ii;
                grid->dist[ii][jj][1] = (double)(grid->zy);
                grid->dist[ii][jj][2] = grid->xval[ii];
                grid->dist[ii][jj][3] = fabs(grid->yval[jj]);
                grid->dist[ii][jj][4] = grid->pcent[ii][3];
            }
        }

    } else {

        for (ii = 0 ; ii < grid->lx ; ii++) {
            jj = 0;

#ifdef DBG
            fprintf(stderr,
                "\r  PlumeDist  ii = %d, lx = %d, ly = %d    ",
                ii,
                grid->lx,
                grid->ly);
#endif
            //---
            // Find the radial distance to each centerline pt
            //  and find the minimum (the closest point)
            //---
            dst1 = dst0;

            for (kk = 0 ; kk < grid->lc ; kk++) {
                rdis = sqrt(sq(grid->pcent[kk][0] - grid->xval[ii])
                        + sq(grid->pcent[kk][1] - grid->yval[jj]));

                if (rdis < dst1) {
                    dst1 = rdis;
                    kin = kk;
                }
            }

            // assign the jj=0,ii=0 dist values
            grid->dist[ii][jj][0] = grid->pcent[kin][0] / grid->dx; // x index, along shore
            grid->dist[ii][jj][1] = grid->pcent[kin][1] / grid->dy; // y index, cross shore
            grid->dist[ii][jj][2] = grid->pcent[kin][2]; // along plume distance
            grid->dist[ii][jj][3] = dst1;                // cross plume distance
            grid->dist[ii][jj][4] = grid->pcent[kin][3]; // centerline velocity

            for (jj = 1 ; jj < grid->ly ; jj++) {
                // find a range of pcent indices around the last point
                k1 = mx(kin - npts, 0);
                k2 = mn(kin + npts + 1, grid->lc);
                // loop through find the closest centerline point
                dst1 = dst0;

                for (kk = k1 ; kk < k2 ; kk++) {
                    rdis = sqrt(sq(grid->pcent[kk][0] - grid->xval[ii])
                            + sq(grid->pcent[kk][1] - grid->yval[jj]));

                    if (rdis < dst1) {
                        dst1 = rdis;
                        kin = kk;
                    }
                }


                // assign the dist values
                grid->dist[ii][jj][0] = grid->pcent[kin][0] / grid->dx; // x index, along shore
                grid->dist[ii][jj][1] = grid->pcent[kin][1] / grid->dy; // y index, cross shore
                grid->dist[ii][jj][2] = grid->pcent[kin][2]; // along plume distance
                grid->dist[ii][jj][3] = dst1;                // cross plume distance
                grid->dist[ii][jj][4] = grid->pcent[kin][3]; // centerline velocity
            }
        }

#ifdef DBG
        fprintf(stderr, "\n");
#endif
    } // endif( fjrd || strt ), else(curved)

#ifdef DBG

    for (kk = 0 ; kk < 5 ; kk++) {
        strcpy(fndbg, "pdst");
        itoa(kk, ss);
        strcat(fndbg, ss);
        strcat(fndbg, ".dat");
        fprintf(stderr, "    Writing dist[ii][jj][%d] to %s \n", kk, fndbg);

        fiddbg = fopen(fndbg, "w");

        for (ii = 0; ii < grid->lx; ii++) {
            for (jj = 0; jj < ly; jj++) {
                fprintf(fiddbg, "%5e  ", grid->dist[ii][jj][kk]);
            }

            fprintf(fiddbg, "\n");
        }

        fclose(fiddbg);
    }

#endif

    return 0;

} // end of PlumeDist

