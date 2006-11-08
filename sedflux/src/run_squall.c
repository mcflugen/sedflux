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
#include "run_squall.h"


Sed_process_info run_squall(gpointer ptr, Sed_cube prof)
{
   Squall_t *data=(Squall_t*)ptr;
   double t=0, dt, total_t;
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

gboolean init_squall(Eh_symbol_table symbol_table,gpointer ptr)
{
   Squall_t *data=(Squall_t*)ptr;
   if ( symbol_table == NULL )
   {
      data->initialized = FALSE;
      return TRUE;
   }

   // Read squall time step.
   data->dt              = eh_symbol_table_time_value( symbol_table , S_KEY_TIME_STEP     );
   // Read squall time fraction.
   data->squall_duration = eh_symbol_table_time_value( symbol_table , S_KEY_TIME_FRACTION );

   return TRUE;
}

