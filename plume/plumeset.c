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
 * PlumeSet.c	Sets various parameters from the input info for Plume
 *
 *
 *	Author:		M.D. Morehead
 *	Original:	Sep 1998
 *	Modified:	Sep 1998, MDM, Conversion for SEDFLUX3D
 *
 */

#include "plumeinput.h"
#include "plumevars.h"
#include <stdio.h>

//---
// outputs:
//    river.b0 : river width
//    river.d0 : river depth
//    dx       : cross-shore grid spacing
//    dy       : along-shore grid spacing
//
// inputs:
//    river.bo : river width
//    river.Q  : river discharge
//    river.u0 : river velocity
//    ndx      : nodes in river mouth
//    ndy      : nodes in cross-shore direction
//---

/*
 *	Start of PlumeSet
 */
int plumeset( Plume_enviro *env , Plume_grid *grid , Plume_options *opt )
{
   int ndx = grid->ndx;
   int ndy = grid->ndy;
   double dx, dy;
   Plume_river *river=env->river;
   Plume_ocean *ocean=env->ocean;

// Constants derived from Input Parameters
// NOTE: we round off b0.  i have no idea why we do this.
   river->b0 = ndy*rnd(river->b0/ndy);

// round so that b0 and dy come out in meters
   river->d0  = river->Q/(river->b0*river->u0);  // river depth (m)
// NOTE: we no longer round to the nearest meter.  dy, and dx are now DOUBLE.
//   dy = (int)(river->b0/ndy);
   dy = river->b0/ndy;                           // along-shore bin spacing (m)
   dx = ndx*dy;                                  // cross-shore bin spacing (m)

   if ( ocean->vo > 0 )
      ocean->vdirection = river->rdirection + M_PI_2;
   else
      ocean->vdirection = river->rdirection - M_PI_2;

//---
// 3D check for direction of kelvin wave propagation
// if the cross product of the Vocean and Vriver is > 0 in the northern
// hemisphere, then the plume is moving in the direction of k.w.p.
// Vo X Vr = |Vo| |Vr| sin( thetao - thetar ) [* perpendicular unit vector]
//---
   if ( !(opt->strt || opt->fjrd ) )
   {
      opt->kwf = 0; 
      if( sin( ocean->vdirection - river->rdirection ) > 0 && env->lat > 0 )
         opt->kwf = 1;
      if( sin( ocean->vdirection - river->rdirection ) < 0 && env->lat < 0 )
         opt->kwf = 1;
   }
//opt->kwf = 1;

   grid->dx = dx;
   grid->dy = dy;

   return(0);
} /* end of PlumeRead */

