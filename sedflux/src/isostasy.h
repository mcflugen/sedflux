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

#ifndef _ISOSTASY_H_
# define _ISOSTASY_H_

# include "utils.h"
# include "sed_sedflux.h"
# include "subside.h"

typedef struct
{
   gboolean initialized;
//   double D; /* the flexural parameter (N m). */
   double relaxation_time; /* relaxation time of the Earth's crust (years) */
   double eet;
   double youngs_modulus;

   double last_time; /* the last time (in years) that the Earth was subsided */
   guint len;
//   double *old_thickness, *old_height;
//   Eh_dbl_grid *old_thickness_grid, *old_height_grid;
   double last_half_load;
   Eh_dbl_grid last_dw_iso, last_load;
}
Isostasy_t;

Sed_process_info run_isostasy(gpointer,Sed_cube);
gboolean init_isostasy(Eh_symbol_table,gpointer);

#endif
