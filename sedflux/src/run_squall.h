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

#ifndef _RUN_SQUALL_H_
# define _RUN_SQUALL_H_

# include "utils.h"
# include "sed_sedflux.h"
# include "squall.h"

typedef struct
{
   gboolean initialized;
   double time_fraction;
   double squall_duration;
   double dt;
}
Squall_t;

Sed_process_info run_squall(gpointer,Sed_cube);
gboolean init_squall(Eh_symbol_table,gpointer);

#endif
