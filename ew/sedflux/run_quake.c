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

#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "my_processes.h"

double earthquake(double,double);

gboolean init_quake_data( Sed_process proc , Sed_cube prof , GError** error );

Sed_process_info
run_quake( Sed_process proc , Sed_cube prof )
{
   Quake_t *    data = (Quake_t*)sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   double a, acceleration, time_step;

   if ( sed_process_run_count(proc)==0 )
      init_quake_data( proc , prof , NULL );

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

gboolean
init_quake( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Quake_t* data    = sed_process_new_user_data( p , Quake_t );
   GError*  tmp_err = NULL;
   gchar**  err_s   = NULL;
   gboolean is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   data->rand       = NULL;

   data->mean_quake = eh_symbol_table_dbl_value( tab , S_KEY_MEAN_QUAKE );
   data->var_quake  = eh_symbol_table_dbl_value( tab , S_KEY_VAR_QUAKE  );
   data->rand_seed  = eh_symbol_table_int_value( tab , S_KEY_SEED       );

   eh_check_to_s( data->mean_quake>=0. , "Magnitude of average earthquake positive" , &err_s );
   eh_check_to_s( data->var_quake>=0.  , "Variance of average earthquake positive"  , &err_s );

   if ( err_s ) eh_set_error_strv( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_PARAM , err_s );

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
init_quake_data( Sed_process proc , Sed_cube prof , GError** error )
{
   Quake_t* data = (Quake_t*)sed_process_user_data( proc );

   if ( data )
   {
      if ( data->rand_seed>0 ) data->rand = g_rand_new_with_seed( data->rand_seed );
      else                     data->rand = g_rand_new( );

      data->last_time = sed_cube_age_in_years( prof );
   }

   return TRUE;
}

gboolean
destroy_quake( Sed_process p )
{
   if ( p )
   {
      Quake_t* data = (Quake_t*)sed_process_user_data( p );
      
      if ( data )
      {
         g_rand_free( data->rand );

         eh_free( data );
      }
   }

   return TRUE;
}

