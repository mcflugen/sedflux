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

#ifndef _BING_IS_INCLUDED
# define _BING_IS_INCLUDED

#include "utils.h"

typedef struct
{
   double U, Uold, Up, Upold;
   double Dbar, Xbar, Ubar, Upbar;
   double D, X, Y; 
} node;

typedef struct
{
   double yieldStrength;
   double viscosity;
   double numericalViscosity;
   double flowDensity;
   double dt;
   double maxTime;
   int MC;
}
bing_t;

#define sq(a) ( (a)*(a) )
#define floor(a) ( (int)(a) )
#define abs(a) ( ((a)>=0)?a:(-1.0*(a)) )
#define sign(a) ( ((a)>0)?1:(((a)==0)?0:(-1)) )
#define PI 3.14159265359

double *bing(pos_t*,pos_t*,bing_t,double*);

#endif /* bing.h is included */
