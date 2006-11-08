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

#define SED_TIDE_PROC_NAME "tide"
#define EH_LOG_DOMAIN SED_TIDE_PROC_NAME

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "utils.h"
#include "tide.h"

Sed_process_info run_tide(gpointer ptr,Sed_cube prof)
{
   Tide_t *data=(Tide_t*)ptr;
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

   if ( data->tidal_period<0 )
      data->tidal_period = sed_cube_time_step_in_days( prof );

   sed_cube_set_tidal_range ( prof , data->tidal_range  );
   sed_cube_set_tidal_period( prof , data->tidal_period );

   eh_message( "time         : %f" , sed_cube_age_in_years(prof) );
   eh_message( "tidal range  : %f" , data->tidal_range                  );
   eh_message( "tidal period : %f" , data->tidal_period                 );

   return info;
}

#include <sys/stat.h>

#define S_KEY_TIDE_RANGE  "tidal range"
#define S_KEY_TIDE_PERIOD "tidal period"

gboolean init_tide( Eh_symbol_table symbol_table,gpointer ptr)
{
   Tide_t *data=(Tide_t*)ptr;
   char *str;

   if ( symbol_table == NULL )
   {
      data->initialized = FALSE;
      return TRUE;
   }

   data->tidal_range  = eh_symbol_table_dbl_value( symbol_table , S_KEY_TIDE_RANGE  );
   str                = eh_symbol_table_lookup   ( symbol_table , S_KEY_TIDE_PERIOD );

   if ( strcasecmp( str , "time step" )==0 )
      data->tidal_period = -1;
   else
   {
      data->tidal_period = g_strtod( str , NULL );
      data->tidal_period = -1;
      g_message( "The tidal period must be set to 'time step'.  Resetting." );
   }

   return TRUE;
}

