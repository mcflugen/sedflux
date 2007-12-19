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
#include <utils/eh_rand.h>

/** Generate an earthquake.

This will give the maximum earthquake that occured within the time step dt.  

@param a   determined from the mean quake over a time step.
@param dt  the number of time steps that are considered to find the maximum.

@return The horizontal ground acceleration due to the earthquake.

*/

double earthquake(double a,double dt)
{
   double acceleration;
   static long seed=124232;

//   acceleration = powdev(a,&seed);
   acceleration = eh_maxpowdev(a,dt,&seed);
   return acceleration;
}

