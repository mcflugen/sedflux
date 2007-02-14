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

#if !defined(AVULSION_H)
#define AVULSION_H

#include <glib.h>
#include "sed_sedflux.h"

#define AVULSION_PROGRAM_NAME  "avlusion"
#define AVULSION_MAJOR_VERSION (1)
#define AVULSION_MINOR_VERSION (0)
#define AVULSION_MICRO_VERSION (0)

typedef struct
{
   GRand* rand;
   double std_dev;
}
Avulsion_st;

#define AVULSION_DATA avulsion_data_struct_quark()

GQuark       avulsion_data_struct_quark    ( void );
double       avulsion                      ( GRand* rand , double last_angle , double std_dev );
Avulsion_st* avulsion_new                  ( GRand* rand , double std_dev );
Avulsion_st* avulsion_dup                  ( Avulsion_st* s );
Avulsion_st* avulsion_destroy              ( Avulsion_st* data );

Sed_riv      sed_river_avulse              ( Sed_riv r  );
Sed_riv      sed_river_set_avulsion_data   ( Sed_riv r , Avulsion_st* data );
Sed_riv      sed_river_unset_avulsion_data ( Sed_riv r );
Sed_riv      sed_river_impart_avulsion_data( Sed_riv r );
Avulsion_st* sed_river_avulsion_data       ( Sed_riv r );
Sed_cube     sed_cube_avulse_river         ( Sed_cube c , Sed_riv r );
Sed_cube     sed_cube_avulse_all_rivers    ( Sed_cube c );

#endif

