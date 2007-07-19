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

#include "sed_sedflux.h"
#include "my_processes.h"

Sed_process_info
run_slump( Sed_process proc , Sed_cube prof )
{
   Slump_t*         data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   int              i;
   Sed_cube         fail;

   fail = sed_process_use( proc , FAILURE_PROFILE_DATA );
   //fail = data->failure;

   // Add the failure to the profile shifted by an amount based
   // on the failure length.
   for ( i=0 ; i<sed_cube_n_y(fail) ; i++ )
      sed_column_set_y_position( sed_cube_col(fail,i) ,
                                   sed_cube_col_y( fail , i )
                                 + (int)(sed_cube_n_y(fail)/2.) );
   sed_cube_add(prof,fail);

   return info;
}

