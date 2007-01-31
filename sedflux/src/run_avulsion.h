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

#if !defined(RUN_AVULSION_H)
# define RUN_AVULSION_H

# include "utils.h"
# include "sed_sedflux.h"

typedef struct
{
   gboolean     initialized;
   Eh_input_val std_dev;
   Eh_input_val min_angle;
   Eh_input_val max_angle;
   Eh_input_val f_remain;
   GRand*       rand;
   guint32      rand_seed;
   gboolean     reset_angle;
   gint         hinge_i;
   gint         hinge_j;
   char*        river_name;
}
Avulsion_t;

Sed_process_info run_avulsion  ( gpointer        , Sed_cube );
gboolean         init_avulsion ( Eh_symbol_table , gpointer );

#endif
