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

#ifndef _RUN_XSHORE_H_
# define _RUN_XSHORE_H_

# include "utils.h"
# include "sed_sedflux.h"
# include "xshore.h"

typedef struct
{
   gboolean initialized;
   double last_time;
   int sediment_type;
   Eh_input_val xshore_current;
}
Xshore_t;

Sed_process_info run_xshore( gpointer ptr , Sed_cube prof );
gboolean init_xshore(Eh_symbol_table,gpointer);

#endif
