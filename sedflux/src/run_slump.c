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

#include "slump.h"
#include "sed_sedflux.h"

Sed_process_info run_slump(gpointer ptr,Sed_cube prof)
{
   Slump_t *data=(Slump_t*)ptr;
   int i;
   Sed_cube fail;
   Sed_process_info info = SED_EMPTY_INFO;

   if ( prof == NULL )
   {
      if ( data->initialized )
         data->initialized = FALSE;
      return SED_EMPTY_INFO;
   }

   if ( !data->initialized )
   {
      data->initialized = TRUE;
   }

   fail = data->failure;

   // Add the failure to the profile shifted by an amount based
   // on the failure length.
   for ( i=0 ; i<sed_cube_n_y(fail) ; i++ )
      sed_column_set_y_position( sed_cube_col(fail,i) ,
                                   sed_cube_col_y( fail , i )
                                 + (int)(sed_cube_n_y(fail)/2.) );
   sed_cube_add(prof,fail);

   return info;
}

gboolean init_slump(Eh_symbol_table symbol_table,gpointer ptr)
{
   Slump_t *data=(Slump_t*)ptr;
   if ( symbol_table == NULL )
   {
      data->initialized = FALSE;
      return TRUE;
   }

   return TRUE;
}


