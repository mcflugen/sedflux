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

#define SED_NEW_PROCESS_PROC_NAME "new process"
#define EH_LOG_DOMAIN SED_NEW_PROCESS_PROC_NAME

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "sed_sedflux.h"
#include "run_new_process.h"
#include "utils.h"
#include "processes.h"

Sed_process_info run_new_process(gpointer ptr,Sed_cube prof)
{
   New_process_t *data=(New_process_t*)ptr;
   Sed_process_info info = SED_EMPTY_INFO;
   
   if ( prof == NULL )
   {
      if ( data->initialized )
      {
         data->initialized = FALSE;
      }
      return SED_EMPTY_INFO;
   }

   if ( !data->initialized )
   {
      data->initialized = TRUE;
   }

   return info;
}

#define S_KEY_PARAM_NAME "parameter"

gboolean init_new_process( Eh_symbol_table symbol_table , gpointer ptr )
{
   New_process_t *data=(New_process_t*)ptr;

   if ( symbol_table == NULL )
   {
      data->initialized = FALSE;
      return TRUE;
   }

   return TRUE;
}

