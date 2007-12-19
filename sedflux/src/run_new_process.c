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

/* Define the name of the process */
#define SED_NEW_PROCESS_PROC_NAME "new process"

/* Set the log domain for this file. */
#define EH_LOG_DOMAIN SED_NEW_PROCESS_PROC_NAME

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sed/sed_sedflux.h>
#include <utils/utils.h>
#include "my_processes.h"

Sed_process_info
run_new_process( Sed_process proc , Sed_cube prof )
{
   New_process_t*   data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;

   /* Do something to the Sed_cube */
   {
      double cur_val = sed_cube_sea_level( prof );
      sed_cube_set_sea_level( prof , cur_val - data->param );
   }

   /* Set and return mass balance information */
   info.mass_added = 0.;
   info.mass_lost  = 0.;

   return info;
}

#define S_KEY_PARAM_NAME_1 "parameter"
#define S_KEY_PARAM_NAME_2 "another parameter"

gboolean
init_new_process( Sed_process p , Eh_symbol_table tab , GError** error )
{
   New_process_t* data    = sed_process_new_user_data( p , New_process_t );
   GError*        tmp_err = NULL;
   gboolean       is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   if ( data )
   {
      /* Get the parameter values from the Eh_symbol_table */

      /* If there is an error, report it */
      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         is_ok = FALSE;
      }
   }

   return is_ok;
}

gboolean
destroy_new_process( Sed_process p )
{
   if ( p )
   {
      New_process_t* data = sed_process_user_data( p );

      if ( data )
      {
         /* Free resources used by data ... */

         /* ... and the data itself */
         eh_free( data );
      }
   }

   return TRUE;
}
