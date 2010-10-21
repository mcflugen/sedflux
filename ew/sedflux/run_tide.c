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

#include <utils/utils.h>
#include "my_processes.h"

Sed_process_info
run_tide( Sed_process proc , Sed_cube prof )
{
   Tide_t*          data = (Tide_t*)sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;

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

gboolean
init_tide( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Tide_t*  data    = sed_process_new_user_data( p , Tide_t );
   GError*  tmp_err = NULL;
   gchar**  err_s   = NULL;
   gboolean is_ok   = TRUE;
   gchar*   str;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   data->tidal_range  = eh_symbol_table_dbl_value( tab , S_KEY_TIDE_RANGE  );
   str                = eh_symbol_table_lookup   ( tab , S_KEY_TIDE_PERIOD );

   if ( g_ascii_strcasecmp( str , "time step" )==0 ) data->tidal_period = -1;
   else
   {
      data->tidal_period = eh_str_to_dbl( str , &tmp_err );

      g_message( "The tidal period must be set to 'time step'.  Resetting." );
      data->tidal_period = -1;
   }

   eh_check_to_s( data->tidal_range>=0 , "Tidal range positive" , &err_s );

   if ( !tmp_err && err_s )
      eh_set_error_strv( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_PARAM , err_s );

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
destroy_tide( Sed_process p )
{
   if ( p )
   {
      Tide_t* data = (Tide_t*)sed_process_user_data( p );
      
      if ( data ) eh_free( data );
   }

   return TRUE;
}

