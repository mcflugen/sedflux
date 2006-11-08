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
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "failure.h"

#ifdef DECIDER_STANDALONE
int main(int argc, char *argv[])
{
   int decider(const Sed_cube*,double);
   int decision;
   Sed_cube p;

   p = sed_cube_read(stdin);

   decision = decider(p,DECIDER_THRESHOLD);

   sed_cube_write(stdout,p);

   fprintf(stderr,"decision = %d\n",decision);

   return decision;
}
#endif

int decider( const Sed_cube p , double threshold )
{
   double fsand=0,fclay=0;
   int decision;

   eh_require( p );
   eh_require( threshold>=0 );
   eh_require( threshold<=1 );
   eh_require( sed_cube_is_1d(p) );

   {
      gssize n_grains = sed_sediment_env_size( );
      gssize i;
      Sed_cell fail = sed_cell_new( n_grains );
      Sed_cell c    = sed_cell_new( n_grains );

      for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
      {
         sed_column_top( sed_cube_col(p,i) , sed_cube_thickness(p,0,i) , c );
         sed_cell_add(fail,c);
      }
      fsand = sed_cell_sand_fraction( fail );
      fclay = sed_cell_clay_fraction( fail );

      sed_cell_destroy( c    );
      sed_cell_destroy( fail );
   }

   if ( fclay >= threshold )
      decision = DECIDER_DEBRIS_FLOW;
   else
      decision = DECIDER_TURBIDITY_CURRENT;

   return decision;
}
