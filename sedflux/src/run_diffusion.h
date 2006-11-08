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

#ifndef _RUN_DIFFUSION_H_
# define _RUN_DIFFUSION_H_

# include "utils.h"
# include "sed_sedflux.h"
# include "diffusion.h"

typedef struct
{
   gboolean initialized;
   Eh_input_val k_max;
   Eh_input_val k_long_max;
   Eh_input_val k_cross_max;
   double skin_depth;
}
Diffusion_t;

Sed_process_info run_diffusion( gpointer ptr , Sed_cube prof );
gboolean init_diffusion(Eh_symbol_table,gpointer);

#endif
