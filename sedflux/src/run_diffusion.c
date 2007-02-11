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
#include "utils.h"
#include "sed_sedflux.h"
#include "run_diffusion.h"
#include "processes.h"

Sed_process_info
run_diffusion( gpointer ptr , Sed_cube prof )
{
   Diffusion_t *data=(Diffusion_t*)ptr;
   double k_max, skin_depth;
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

   // Adjust diffusion constants for storms.
/*
   k_long_max  = eh_eval_input_val( data->k_long_max ,
                                    sed_get_cube_age_in_years( prof ) )
               * sed_get_cube_storm(prof);
   k_cross_max = eh_eval_input_val( data->k_cross_max ,
                                    sed_get_cube_age_in_years( prof ) )
               * sed_get_cube_storm(prof);
*/

   k_max       = eh_input_val_eval( data->k_max ,
                                    sed_cube_age_in_years( prof ) )
               * sed_cube_storm(prof);
   skin_depth  = data->skin_depth*sed_cube_storm(prof);
   
   eh_require( k_max>0 )
   {
      Sed_cell *lost;

      eh_message( "time                  : %f" ,
                  sed_cube_age_in_years(prof) );

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


      eh_message( "time step (days)     : %f" ,
                  sed_cube_time_step_in_days(prof) );
      eh_message( "diffusion coeficient : %f" , k_max                     );
   }

   return info;
}

#define S_KEY_K_MAX         "diffusion constant"
#define S_KEY_K_LONG_MAX    "long-shore diffusion constant"
#define S_KEY_K_CROSS_MAX   "cross-shore diffusion constant"
#define S_KEY_SKIN_DEPTH    "diffusion 1% depth"

gboolean
init_diffusion( Eh_symbol_table tab , gpointer ptr )
{
   Diffusion_t *data=(Diffusion_t*)ptr;
   GError* err = NULL;

   if ( tab == NULL )
   {
      eh_input_val_destroy( data->k_max );
      data->initialized = FALSE;
      return TRUE;
   }

   if ( (data->k_max = eh_symbol_table_input_value(tab,S_KEY_K_MAX ,&err)) == NULL )
   {
      fprintf( stderr , "Unable to read input values: %s" , err->message );
      eh_exit( EXIT_FAILURE );
   }

   data->skin_depth = eh_symbol_table_dbl_value( tab , S_KEY_SKIN_DEPTH );

   return TRUE;
}

