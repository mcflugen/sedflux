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

#define SED_QUAKE_PROC_NAME "earthquake"
#define EH_LOG_DOMAIN SED_QUAKE_PROC_NAME

#include <stdio.h>

#include "utils.h"
#include "quake.h"
#include "sed_sedflux.h"

Sed_process_info run_quake(gpointer ptr,Sed_cube prof)
{
   Quake_t *data=(Quake_t*)ptr;
   double earthquake(double,double);
   double a, acceleration, time_step;
   Sed_process_info info = SED_EMPTY_INFO;

   if ( prof == NULL )
   {
      if ( data->initialized )
      {
         data->initialized = FALSE;
         g_rand_free( data->rand );
      }
      return SED_EMPTY_INFO;
   }
   
   if ( !data->initialized )
   {
      if ( data->rand_seed>0 )
         data->rand = g_rand_new_with_seed( data->rand_seed );
      else
         data->rand = g_rand_new( );

      data->last_time = sed_cube_age_in_years(prof);
      data->initialized = TRUE;
   }

   a               = exp(-1./data->mean_quake);
   time_step       = sed_cube_age_in_years( prof ) - data->last_time;
   data->last_time = sed_cube_age_in_years( prof );

   //---
   // Draw the acceleration value from a density function based on the
   // 100 year earthquake.
   //
   // Note that the time step will be 0 at the start of each epoch.
   //---
   if ( time_step > 1e-6 )
      acceleration = eh_max_log_normal( data->rand       ,
                                        data->mean_quake ,
                                        data->var_quake  ,
                                        time_step/100.   );
   else
      acceleration = 0.;

   sed_cube_set_quake(prof,acceleration);

   eh_message( "time         : %f" , sed_cube_age_in_years(prof) );
   eh_message( "time step    : %f" , time_step                   );
   eh_message( "acceleration : %g" , acceleration                );

   return info;
}

#include <stdlib.h>
#include <time.h>

//#define S_KEY_MEAN_QUAKE "mean maximum yearly earthquake"
#define S_KEY_MEAN_QUAKE "mean acceleration of 100 year quake"
#define S_KEY_VAR_QUAKE  "variance of 100 year quake"
#define S_KEY_SEED       "seed for random number generator"

gboolean init_quake( Eh_symbol_table symbol_table,gpointer ptr)
{
   Quake_t *data=(Quake_t*)ptr;

   if ( symbol_table == NULL )
   {
      data->initialized=FALSE;
      return TRUE;
   }

   data->mean_quake = eh_symbol_table_dbl_value( symbol_table , S_KEY_MEAN_QUAKE );
   data->var_quake  = eh_symbol_table_dbl_value( symbol_table , S_KEY_VAR_QUAKE  );
   data->rand_seed  = eh_symbol_table_int_value( symbol_table , S_KEY_SEED       );

   return TRUE;
}

