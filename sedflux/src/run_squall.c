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

#define SED_SQUALL_PROC_NAME "squall"
#define EH_LOG_DOMAIN SED_SQUALL_PROC_NAME

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "squall.h"
#include "my_processes.h"

Sed_process_info
run_squall( Sed_process proc , Sed_cube prof )
{
   Squall_t *       data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   double t=0, dt, total_t;

// 1e6y year run
//   total_t = sed_get_profile_time_step_in_years( prof )*1e-2;
// squall run
//   total_t = sed_get_profile_time_step_in_years( prof )*1e-0;

//   total_t = sed_get_profile_time_step_in_years( prof )*data->time_fraction;
   total_t = data->squall_duration;
   dt      = data->dt;

//   sed_set_profile_wave_height( prof , 9   );
//   sed_set_profile_wave_length( prof , 260 );
   
   eh_message( "squall            : %s" , "start" );
   eh_message( "time              : %f" , sed_cube_age_in_years(prof) );
   eh_message( "time step         : %f" , data->dt                    );
   eh_message( "no. of time steps : %f" , total_t/data->dt            );
   eh_message( "wave height       : %f" , sed_cube_wave_height(prof)  );
   eh_message( "wave length       : %f" , sed_cube_wave_length(prof)  );
   eh_message( "wave period       : %f" , sed_cube_wave_period(prof)  );

   do
   {
      if ( t+dt>total_t )
         dt = total_t-t;
      if ( !squall( prof , dt ) )
      {
         eh_message( "squall            : %s" , "done" );
         return info;
      }
      t += dt;
   }
   while ( t<total_t );

   eh_message( "squall            : %s" , "done" );

   return info;
}

#define S_KEY_TIME_STEP      "time step"
#define S_KEY_TIME_FRACTION  "duration of squall"

gboolean
init_squall( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Squall_t* data    = sed_process_new_user_data( p , Squall_t );
   GError*   tmp_err = NULL;
   gchar**   err_s   = NULL;
   gboolean  is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   // Read squall time step and time fraction.
   data->dt              = eh_symbol_table_time_value( tab , S_KEY_TIME_STEP     );
   data->squall_duration = eh_symbol_table_time_value( tab , S_KEY_TIME_FRACTION );

   eh_check_to_s( data->dt>=0.               , "Time step positive"          , &err_s );
   eh_check_to_s( data->squall_duration>=0.  , "Duration of squall positive" , &err_s );

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
destroy_squall( Sed_process p )
{
   if ( p )
   {
      Squall_t* data = sed_process_user_data( p );
      
      if ( data ) eh_free( data );
   }

   return TRUE;
}

