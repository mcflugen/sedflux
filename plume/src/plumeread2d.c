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
 * PlumeRead2D.c	Reads the 2D input file for Plume
 *
 *
 *	Author:		M.D. Morehead
 *	Original:	April 1998
 *	Modified:	Sep 1998, MDM, Conversion for SEDFLUX3D
 *
 */
#include "plumeinput.h"
#include "plumevars.h"
#include <stdio.h>

int plumeread2d(char *fileName , Plume_enviro *env , Plume_grid *grid , Plume_options *opt )
{
   static int first_time=1;
   FILE *fidin;
   char chs[120];
   Plume_ocean ocean = *(env->ocean);

   if ( first_time )
   {
      // Open the input file
      if( (fidin = fopen(fileName,"r")) == NULL)
      {
         fprintf(stderr, "  PlumeRead ERROR: Unable to open the input file: %s \n",fileName);
         eh_exit(1);
      }

      // Ocean parameters.
      fscanf( fidin, "%lf", &ocean.Cw);    fgets( chs, 120, fidin );
      fscanf( fidin, "%lf", &ocean.vo);    fgets( chs, 120, fidin );
      fscanf( fidin, "%lf", &ocean.vdirection); fgets( chs, 120, fidin );
      fscanf( fidin, "%lf", &ocean.cc);    fgets( chs, 120, fidin );
      fscanf( fidin, "%lf", &ocean.So);    fgets( chs, 120, fidin );
      fscanf( fidin, "%lf", &ocean.Sw);    fgets( chs, 120, fidin );

      // Other Parameters
      // line 14-16
      fscanf( fidin, "%1d", &opt->fjrd);   fgets( chs, 120, fidin );
      fscanf( fidin, "%1d", &opt->kwf);    fgets( chs, 120, fidin );
      fscanf( fidin, "%d" , &env->lat);    fgets( chs, 120, fidin );

      // grids
      // lines 17-18
      fscanf( fidin, "%2d", &grid->ndy);   fgets( chs, 120, fidin );
      fscanf( fidin, "%2d", &grid->ndx);   fgets( chs, 120, fidin );

      // debug and output option flags
      // line 21, dbg, removed
      // lines 22-24
      fscanf( fidin, "%d", &opt->o1);      fgets( chs, 120, fidin );
      fscanf( fidin, "%d", &opt->o2);      fgets( chs, 120, fidin );
      fscanf( fidin, "%d", &opt->o3);

      fclose(fidin);
      first_time=0;
   }

   return(0);

} // end of PlumeRead

