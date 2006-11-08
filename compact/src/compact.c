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
#include <math.h>
#include "utils.h"
#include "sed_sedflux.h"

/** \file compact.c

   Sediment compaction

   \defgroup compaction_group Sediment Compaction

   \callgraph
*/
/*@{*/

/** Compact a column of sediment

Compact a column of sediment.  The amount that a cell of sediment
is compacted depends upon the load paced on it from the sediment
vertically above it.  That is,

   \f[
      { \partial \phi \over \partial \sigma} = -c \left( \phi - \phi_0 \right)
   \f]

where \f$ \phi \f$ is porosity, \f$ \phi_0 \f$ is closest-packed porosity,
\f$ \sigma \f$ is the overlying load, and \f$ c \f$ is the compaction
coefficient.

Each cell of sediment keeps track of the thickness that it would be
if it was not compacted.  So, to compact a column of sediment we
act only on the uncompacted thickness of each cell.  This way we
don't have to worry about compacting sediment that has already been
compacted.  Also, if a compacted cell of sediment is thicker than
before compaction, then overlying sediment has been removed and the
thickness of the cell is not changed.

\param s        A Sed_column to compact
\param time_now The current time

*/

int compact( Sed_column s )
{
   double *load_eff;
   double hydro_static;
   gssize n_grains = sed_sediment_env_size();

   // If there is only one (or no) bins, there is no
   // overlying load and so no compaction.
   if ( sed_column_len(s) < 2 )
      return 0;

   load_eff     = eh_new0( double , sed_column_len(s) );
   hydro_static = sed_column_water_pressure( s );

   // Calculate the load of the overlying sediment.
   {
      gssize i;
      double p;
      double* load = sed_column_load( s , 0 , -1 , NULL );

      for (i=sed_column_len(s)-1; i>=0; --i) 
      {
         p = sed_cell_pressure( sed_column_nth_cell( s , i ) );

         load_eff[i] = load[i] - p;

         if ( load_eff[i] < 0 )
            load_eff[i] = 0.;
      }

      eh_free( load );
   }

   // Calculate the densities that the sediment should be.
   {
      gssize i, n;
      Sed_cell this_cell;
      double t_new;
      double* c         = sed_sediment_property( NULL , &sed_type_compressibility );
      double* rho_grain = sed_sediment_property( NULL , &sed_type_rho_grain       );
      double* rho_max   = sed_sediment_property( NULL , &sed_type_rho_max         );
      double* rho       = sed_sediment_property( NULL , &sed_type_rho_sat         );
      double rho_new;
      double t_0;

      for (i=sed_column_len(s)-1; i>=0 ; i--)
      {
         this_cell = sed_column_nth_cell( s , i );
         for ( n=0 , t_new=0. ; n<n_grains ; n++ )
         {
            t_0 = sed_cell_sediment_volume(this_cell)*sed_cell_fraction(this_cell,n);
            rho_new = rho_max[n] + (rho[n]-rho_max[n]) / exp(c[n]*load_eff[i]);

            t_new += t_0*(rho_grain[n]-sed_rho_sea_water())/(rho_new-sed_rho_sea_water());

            if ( t_new< 0 )
            {
               t_new = 0;
               eh_require_not_reached();
            }

         }

         // If the new thickness is greater than the current thickness, we
         // don't do anything.  In this case overlying sediment has been eroded.
         if ( t_new < sed_cell_size(this_cell) )
            sed_column_compact_cell(s,i,t_new);
      }

      eh_free( c         );
      eh_free( rho_max   );
      eh_free( rho_grain );
      eh_free( rho       );
   }

   eh_free(load_eff);
   
   return 0;
}

/*@}*/
