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

#if !defined(RUN_PLUME_H)
# define RUN_PLUME_H

# include "utils.h"
# include "sed_sedflux.h"

#include "plume_types.h"

typedef struct
{
   gboolean initialized;

   Eh_input_val current_velocity;
   double ocean_concentration;
   double plume_width;
   int ndx;
   int ndy;

   int deposit_size;
   Sed_cell **deposit;
   Sed_cell **last_deposit;
   double **plume_deposit;
   Plume_river last_river_data;
   Plume_data *plume_data;

   Sed_cell_grid deposit_grid;
   Sed_cell_grid last_deposit_grid;
}
Plume_t;

Sed_process_info run_plume(gpointer,Sed_cube);
gboolean init_plume(Eh_symbol_table,gpointer);

#endif
