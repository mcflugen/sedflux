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

#include "plumevars.h"
#include "plumeinput.h"

#include <utils/utils.h>

//grid_type_3d *grid;
//river_type river;
//int     ngrains;        /* actual number of grains simulated */
//sedload_type *sedload;
//ocean_type ocean;

/*
 *      Other Parameters
 */
//int     fjrd;   /* Flag for Fjord conditions, fjrd = 1; coastal, fjrd = 0 */
//int     kwf;    /* Kelvin Wave Flag, 0 = not, 1 = yes   */
//double  lat;    /* latitude north in degrees [0:90]     */

/*
 *      Set up grids (ymin,ymax used in Fjord runs (fr == 0) and for vo == 0
 */
//int     ndy;    /* N points within rivermouth (used for dy), must be odd!!
//                 *   (11 is a good number, reducing mass balance errors) */
//int     ndx;    /* x-grid spacing = ndx * dy
//                 *   (open ocean use 1, fjords use 3 to 10)     */
//int     dy, dx; /* actual bin spacing [calculated]              */
//double  ymin;   /* Y (alongshore) range (m)     [< -2*bo]       */
//double  ymax;   /*                              [>  2*bo]       */
//double  xmin;   /* X (crossshore) range (m)     [0]     */
//double  xmax;   /*                              [lots]  */
//int     o1, o2, o3;     /* output flags for 1=standalone, 2=SEDFLUX-2D, 3=SEDFLUX-3D */

//int     strt;                   /* Straight plume flag, if straight (or Fjord) = 1, else = 0 */


FILE    *fidlog;                /* Log file */

//---
// Grid data
//---
//double ***ccnc, ***ncnc, ***deps, ***dist;
//double **ualb, **pcent, **sln;
//double *xval, *yval;
//int lc, lpc, lx, ly, zx, zy;

//---
// Mass balance
//---
//double Qsr, Qsw[4], Tsr, Tsd[2], merr;

int
plume( Plume_enviro *env , Plume_grid *grid , Plume_options *opt )
{
   int     err;
   Plume_mass_bal mb;

   // Constants derived from Input Parameters
   err = plumeset( env , grid , opt );

   // Check for Hydraulic Jumps
   err = plumejump( env->river );

   // Set flag for fjords or Straight plumes

   if ( opt->fjrd || fabs(env->ocean->vo) < ucrit*env->river->u0 )
      opt->strt = 1;                    // assume: vo ~ 0
   else
      opt->strt = 0;
/*
   if ( opt->fjrd )
      opt->strt = 1;
*/

   // Check input values
   err = plumecheck( env , grid , opt );
   if( err )
   {
//      fprintf( stderr, " PlumeCheck ERROR: Plume Aborted \n\n" );
      if ( opt->o1 )
           fprintf( fidlog, " PlumeCheck ERROR: Plume Aborted \n\n" );
      return -1;
   }

   // Create nicely centered X/Y arrays
   eh_return_val_if_fail( plumearray( grid , env , opt )==0 , -1 );

   // Calculate centerline position and distance from mouth
   eh_return_val_if_fail( plumecent( env , grid , opt )==0  , -1 );

   // Calculate distance from/along centerline for all other pts
   eh_return_val_if_fail( plumedist( grid , opt )==0        , -1 );

   // Calculate the plume concentrations
   eh_return_val_if_fail( plumeconc( env , grid , opt )==0  , -1 );

   // Mass Conservation Checks and corrections
   eh_return_val_if_fail( plumemass( env , grid , &mb )==0  , -1 );

   return 0;

}

static gchar* _default_config[] = {
"background ocean concentration",
"velocity of coastal current",
"maximum plume width",
"number of grid nodes in cross-shore",
"number of grid nodes in river mouth",
NULL
};

gchar*
get_config_text (const gchar* file)
{
  if (g_ascii_strcasecmp (file, "config")==0)
    return g_strjoinv ("\n", _default_config);
  else
    return NULL;
}

