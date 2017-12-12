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

#define SED_DIFFUSION_PROC_NAME "diffusion"
#define EH_LOG_DOMAIN SED_DIFFUSION_PROC_NAME

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include <diffusion.h>
#include "my_processes.h"

#include "sedflux.h"

Sed_process_info
run_diffusion( Sed_process proc , Sed_cube prof )
{
   Diffusion_t*     data = (Diffusion_t*)sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   double           k_max;
   double           skin_depth;

   // Adjust diffusion constants for storms.
/*
   k_long_max  = eh_eval_input_val( data->k_long_max ,
                                    sed_get_cube_age_in_years( prof ) )
               * sed_get_cube_storm(prof);
   k_cross_max = eh_eval_input_val( data->k_cross_max ,
                                    sed_get_cube_age_in_years( prof ) )
               * sed_get_cube_storm(prof);
*/

   k_max       = eh_input_val_eval( data->k_max , sed_cube_age_in_years( prof ) )
               * sed_cube_storm(prof);
   skin_depth  = data->skin_depth
               * sed_cube_storm(prof);
   
   eh_require(k_max >= 0)
   eh_require(skin_depth >= 0)
   {
      Sed_cell *lost;

      if ( sed_mode_is_3d() )
         lost = diffuse_sediment_2(
                   prof       , k_max , k_max                    ,
                   skin_depth , sed_cube_time_step_in_days(prof) ,
                   DIFFUSION_OPT_WATER );
      else
         lost = diffuse_sediment(
                   prof       , k_max                            ,
                   skin_depth , sed_cube_time_step_in_days(prof) ,
                   DIFFUSION_OPT_WATER );

      if ( lost )
      {
         int i;
         for ( i=0 ; i<4 ; i++ )
            sed_cell_destroy( lost[i] );
         eh_free( lost );
      }


   }

   eh_message( "time                 : %f" , sed_cube_age_in_years(prof)      );
   eh_message( "time step (days)     : %f" , sed_cube_time_step_in_days(prof) );
   eh_message( "diffusion coeficient : %f" , k_max                            );

   return info;
}

#define DIFFUSION_KEY_K_MAX         "diffusion constant"
#define DIFFUSION_KEY_SKIN_DEPTH    "diffusion 1% depth"
#define DIFFUSION_KEY_K_LONG_MAX    "long-shore diffusion constant"
#define DIFFUSION_KEY_K_CROSS_MAX   "cross-shore diffusion constant"

static const gchar* diffusion_req_labels[] =
{
   DIFFUSION_KEY_K_MAX       ,
   DIFFUSION_KEY_SKIN_DEPTH  ,
   NULL
};

gboolean
init_diffusion( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Diffusion_t* data    = sed_process_new_user_data( p , Diffusion_t );
   GError*      tmp_err = NULL;
   gchar**      err_s   = NULL;
   gboolean     is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   eh_symbol_table_require_labels( tab , diffusion_req_labels , &tmp_err );

   if ( !tmp_err )
   {
      data->k_max      = eh_symbol_table_input_value( tab , DIFFUSION_KEY_K_MAX , &tmp_err );
      data->skin_depth = eh_symbol_table_dbl_value  ( tab , DIFFUSION_KEY_SKIN_DEPTH );

      // eh_check_to_s(data->k_max > 0., "Diffusion coefficient positive", &err_s);
      eh_check_to_s(data->skin_depth > 0., "Skin depth positive", &err_s);

      if ( !tmp_err && err_s )
         eh_set_error_strv( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_PARAM , err_s );
   }

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
destroy_diffusion( Sed_process p )
{
   if ( p )
   {
      Diffusion_t* data = (Diffusion_t*)sed_process_user_data( p );

      if ( data )
      {
         eh_input_val_destroy( data->k_max );
         eh_free( data );
      }
   }
   return TRUE;
}
