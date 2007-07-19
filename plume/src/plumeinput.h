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

#ifndef __PLUMEINPUT_IS_INCLUDED__
#define __PLUMEINPUT_IS_INCLUDED__
/*
 *	PlumeInput.h		Type definitions for the input variables
 *
 *	Author:		M.D. Morehead
 *	Original:	April 1998
 *	Modified:	Sep 1998, MDM, Conversion for SEDFLUX3D
 */	

/***************************************************************/
/*	this is a type definition for grid data
 *	Also defined in PLUME's plumeinput.h
 */

#include <stdlib.h>
#include "utils.h"
#include "plume_types.h"

//extern grid_type_3d *grid;

/*
 *	River parameters
 *	Also defined in SEDFLUX3D's typedef.h
 */
#ifdef EXCLUDE
typedef struct { 
   double *Cs;		/* River Concentration (kg/m^3) [0.001:100]	*/
   double Q;			/* discharge (m^3/s) [1:1e6]		*/
   double u0;		/* velocity (m/s) [0.01:10]		*/
   double rdirection;	/* river mouth direction (degN) [0:360]	*/
   double b0;		/* river mouth width (m) [1.0:1e5]	*/
   double d0;		/* river depth (m) [calculated]		*/
   double rma;		/* River mouth angle (degrees), + is in plus y dir. */
} river_type;
#endif

//extern river_type river;

/*
 *	Sediment Load Parameters
 *	Also defined in SEDFLUX3D's typedef.h
 */
//extern int	ngrains;	/* actual number of grains simulated */

#ifdef EXCLUDE
typedef struct {
   double lambda;	/* removal rate coefficient, input (1/day) [0.1:40] */
  			/* immediately convert to (1/s)			*/
   double rho;	/* density of sediment (kg/m^3) [1100:2600]	*/
   double grainsize;	/* used by SEDFLUX3D				*/
   double diff_coef;	/* used by SEDFLUX3D				*/
} sedload_type;
#endif

//extern sedload_type *sedload;

/*
 *	Ocean Parameters
 *	Also defined in SEDFLUX3D's typedef.h
 */
#ifdef EXCLUDE
typedef struct
{
   double Cw;		/* ocean sediment concentration (kg/m^3) [0:min(Cs)]	*/
   double vo;		/* alongshore current magnitude (m/s) [-3:3]	*/
   double vdirection;	/* alongshore current direction (degN) [0:360]	*/
   double cc;		/* Coastal Current width = cc*inertial length scale [0.1:1] */
   double So;		/* Conservative Tracer Property, River concentration	*/
   double Sw;		/* Conservative Tracer Property, Ocean concentration	*/
} ocean_type;
#endif

//extern ocean_type ocean;

/*
 *	Other Parameters
 */
//extern int	fjrd;	/* Flag for Fjord conditions, fjrd = 1; coastal, fjrd = 0 */
//extern int	kwf;	/* Kelvin Wave Flag, 0 = not, 1 = yes	*/
//extern double	lat;	/* latitude north in degrees [0:90]	*/

/*
 *	Set up grids (ymin,ymax used in Fjord runs (fr == 0) and for vo == 0
 */
//extern int	ndy;	/* N points within rivermouth (used for dy), must be odd!!
//		 *   (11 is a good number, reducing mass balance errors) */
//extern int	ndx;	/* x-grid spacing = ndx * dy
//		 *   (open ocean use 1, fjords use 3 to 10)	*/
//extern int	dy, dx;	/* actual bin spacing [calculated]		*/
//extern double	ymin;	/* Y (alongshore) range (m)	[< -2*bo]	*/
//extern double	ymax;	/* 				[>  2*bo]	*/
//extern double	xmin;	/* X (crossshore) range (m)	[0]	*/
//extern double	xmax;	/* 				[lots]	*/
//extern int	o1, o2, o3;	/* output flags for 1=standalone, 2=SEDFLUX-2D, 3=SEDFLUX-3D */

/* Function declarations.
*/
int plume( Plume_enviro* , Plume_grid* , Plume_options* );
int plumeread2d(char* , Plume_enviro* , Plume_grid* , Plume_options* );
int plumeset( Plume_enviro* , Plume_grid* , Plume_options* );
int plumejump( Plume_river* );
int plumecheck( Plume_enviro* , Plume_grid* , Plume_options* );
int plumearray( Plume_grid* , Plume_enviro* , Plume_options* );
int plumecent( Plume_enviro* , Plume_grid* , Plume_options* );
int plumedist( Plume_grid* , Plume_options* );
int plumeconc( Plume_enviro* , Plume_grid* , Plume_options* );
int plumemass( Plume_enviro* , Plume_grid* , Plume_mass_bal* );
int plumeout2( Plume_enviro* , Plume_grid* , double , double** , int , int , double );
int plumeout3( Plume_enviro *env , Plume_grid *grid , Eh_dbl_grid *deposit_grid );
int plumelog( Plume_enviro* , Plume_grid* , Plume_options* , Plume_mass_bal* );

Plume_data *plume_data_init( Plume_data* );
void destroy_plume_data( Plume_data* );

#endif // plumeinput.h is included
