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

#ifndef _BBL_H_
# define _BBL_H_

# include "utils.h"
# include "sed_sedflux.h"

#define BBL_ALGORITHM_NONE 0
#define BBL_ALGORITHM_MUDS 1

typedef struct
{
   int algorithm;
   gboolean initialized;
   gchar* src_file;
   Eh_sequence* src_seq;
   double last_year;
}
Bbl_t;

Sed_process_info run_bbl(gpointer,Sed_cube);
gboolean init_bbl(Eh_symbol_table,gpointer);

#endif
