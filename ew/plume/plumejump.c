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
 * PlumeJump.c  Check for hydraulic jumps
 *      Corrects uo, do, bo if necessary
 *
 * Input:
 *  river.u0    initial river mouth velocity
 *  river.d0    initial river mouth depth
 *  river.b0    initial river mouth width
 *
 * Checks for hydraulic jumps based on the Froude Number
 *  Fr = uo/sqrt(g*do) > 1
 * Then recalculates:
 *  uo and do
 * assumes the changes are small so that bo does not change
 *
 *  Author:     M.D. Morehead
 *  Original:   April 1998
 *
 */
#include "plumeinput.h"
#include "plumevars.h"

//---
// outputs:
//   river.d0 : river depth
//   river.u0 : river velocity
// inputs:
//   river.d0 : river depth
//   river.u0 : river velocity
//---

int
plumejump(Plume_river* river)
{
    double froude, d1, u1;

    froude = river->u0 / sqrt(grv * river->d0);

    if (froude > 1.0) {
        d1 = -(river->d0 / 2)
            + sqrt(sq(river->d0 / 2) + ((2 * river->d0 * sq(river->u0)) / grv));
        u1 = river->u0 * river->d0 / d1;

        fprintf(stderr, " PlumeJump: Hydraulic Jump detected and fixed \n");
        fprintf(fidlog, " PlumeJump: Hydraulic Jump detected and fixed \n");
        fprintf(fidlog, "   Fr   = %g \n", froude);
        fprintf(fidlog, "   Uold = %g \n", river->u0);
        fprintf(fidlog, "   Unew = %g \n", u1);
        fprintf(fidlog, "   Dold = %g \n", river->d0);
        fprintf(fidlog, "   Dnew = %g \n\n", d1);

        river->u0 = u1;
        river->d0 = d1;
    }

#ifdef DBG
    fprintf(fidlog, " PlumeJump: \n");
    fprintf(fidlog, "   Fr   = %g \n", froude);
    fprintf(fidlog, "   U    = %g \n", river->u0);
    fprintf(fidlog, "   D    = %g \n", river->d0);
#endif

    return 0;

} // end PlumeJump

