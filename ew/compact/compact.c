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
#include <utils/utils.h>
#include <sed/sed_sedflux.h>

#ifdef HAVE_OMP_H
# include <omp.h>
#endif

/** \file compact.c

   Sediment compaction

   \defgroup compaction_group Sediment Compaction

   \callgraph
*/
/*@{*/

GQuark
compact_error_quark( void )
{
   return g_quark_from_static_string( "compact-error-quark" );
}

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

*/
gint
compact( Sed_column s )
{
   eh_require( s );

   if ( s || sed_column_len(s)<2 )
   { /* There is a column with overlying load; compact it! */
      const double hydro_static = sed_column_water_pressure( s );
      const gint n_grains = sed_sediment_env_n_types();
      const gint col_len  = sed_column_len(s);
      double*    load_eff = eh_new0( double , col_len );

      eh_require( load_eff );

      { /* Calculate the load of the overlying sediment. */
         gint  i;
         double* load = sed_column_load    ( s , 0 , -1 , NULL );
         double* p    = sed_column_pressure( s , 0 , -1 , NULL );

         for ( i=0; i<col_len; i++ )
         {
            load_eff[i] = load[i] - p[i];
            if ( load_eff[i] < 0 )
               load_eff[i] = 0.;
         }
/*
         for (i=sed_column_len(s)-1; i>=0; --i) 
         {
            p = sed_cell_pressure( sed_column_nth_cell( s , i ) );

            load_eff[i] = load[i] - p;

            if ( load_eff[i] < 0 )
               load_eff[i] = 0.;
         }
*/
         eh_free( p    );
         eh_free( load );
      }

      { /* Calculate the densities that the sediment should be. */
         gint i, n;
         Sed_cell this_cell;
         double t_new;
         double* c         = sed_sediment_property( NULL,
                                                 &sed_type_compressibility );
         double* rho_grain = sed_sediment_property( NULL, &sed_type_rho_grain       );
         double* rho_max   = sed_sediment_property( NULL, &sed_type_rho_max         );
         double* rho       = sed_sediment_property( NULL, &sed_type_rho_sat         );
         double rho_new;
         double t_0;
         const double rho_sea_water = sed_rho_sea_water();

      for (i=sed_column_len(s)-1; i>=0 ; i--)
      { /* From the top of the column to the bottom. */

         this_cell = sed_column_nth_cell( s , i );
         for ( n=0 , t_new=0. ; n<n_grains ; n++ )
         { /* Compact each grain type. */

            t_0 = sed_cell_sediment_volume(this_cell)
                * sed_cell_fraction(this_cell,n);
            rho_new = rho_max[n] + (rho[n]-rho_max[n]) / exp(c[n]*load_eff[i]);

            t_new += t_0*(rho_grain[n]-rho_sea_water)/(rho_new-rho_sea_water);

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
   }
   
   return 0;
}

gboolean
compact_cube( Sed_cube p )
{
   gboolean success = FALSE;

   eh_require( p );

   if ( p )
   {
      gint i;
      const gint len = sed_cube_size(p);

#pragma omp parallel for num_threads(4)
      for ( i=0 ; i<len ; i++ )
      {
         Sed_column s = sed_cube_col( p, i );
         compact( s );
      }

      success = TRUE;
   }
   return success;
}

double
compact_sediment( double t_sed, double load,
                  double rho_grain, double rho_max, double rho, double rho_void,
                  double c ) 
{
   double rho_new = rho_max + (rho-rho_max) * exp( -c*load );
   return t_sed*(rho_grain-rho_void)/(rho_new-rho_void);
}

/*@}*/
