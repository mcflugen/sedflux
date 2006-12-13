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

#define SED_XSHORE_PROC_NAME "xshore"
#define EH_LOG_DOMAIN SED_XSHORE_PROC_NAME

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "run_xshore.h"
#include "processes.h"

gboolean is_worth_running( Sed_ocean_storm s );

Sed_process_info
run_xshore( gpointer ptr , Sed_cube prof )
{
   Xshore_t *data=(Xshore_t*)ptr;
   Sed_process_info p_info = SED_EMPTY_INFO;
   double dt;
   double start_time, end_time, current_time;
   double xshore_current;
   double this_sea_level;
   double mass_before, mass_after, mass_added, mass_lost;
   Sed_cell *lost = NULL;
   Sed_cell along_shore_sediment;
   Sed_ocean_storm this_storm;
   GSList *this_link;
   Xshore_info info;

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
      data->last_time = sed_cube_age_in_years( prof );
      data->initialized = TRUE;
   }

   if ( is_sedflux_3d() )
      return p_info;
   
   start_time      = data->last_time;
   end_time        = sed_cube_age_in_years( prof );
   dt              = end_time - start_time;
   data->last_time = sed_cube_age_in_years( prof );

   {
      double *along_shore_fraction = eh_new0( double , sed_sediment_env_size() );

      along_shore_fraction[data->sediment_type] = 1.;

      along_shore_sediment = sed_cell_new_env( );
      sed_cell_set_fraction( along_shore_sediment ,
                             along_shore_fraction );

      eh_free( along_shore_fraction );
   }

//   dt = sed_cube_time_step( prof );

   current_time = start_time;
   sed_cube_set_age( prof , start_time );

   for ( this_link=sed_cube_storm_list( prof ) ;
         this_link ;
         this_link=this_link->next )
   {
      double storm_wave;
      double max_current = eh_input_val_eval( data->xshore_current ,
                                              current_time );
      mass_before = sed_cube_mass( prof );

      this_storm = this_link->data;

      storm_wave = sed_ocean_storm_wave_height( this_storm );

      xshore_current = max_current*0.5*storm_wave;
      eh_clamp( xshore_current , 0 , max_current );

      this_sea_level = sed_cube_sea_level( prof );
      sed_cube_adjust_sea_level( prof , storm_wave*.5 );

//   if ( sed_cube_time_step_in_days( prof ) > 3. )
//      sed_cube_set_time_step( prof , 3./S_DAYS_PER_YEAR );

//   if ( sed_cube_wave_height( prof ) > .1 )
      if ( is_worth_running( this_storm ) )
      {
         info = xshore( prof                 ,
                        along_shore_sediment ,
                        xshore_current       ,
                        this_storm );
      }
      else
      {
         info.added = NULL;
         info.lost  = NULL;
         info.dt    = NULL;
      }

      sed_cube_set_sea_level( prof , this_sea_level );

      eh_message( "time                            : %f" , current_time );

      if ( TRUE )
      {
         if ( info.added )
         {
            mass_added = sed_cell_mass( info.added )
                       * sed_cube_x_res( prof )
                       * sed_cube_y_res( prof );
            mass_lost  = sed_cell_mass( info.lost )
                       * sed_cube_x_res( prof )
                       * sed_cube_y_res( prof );
//            sed_cell_resize( info.added , sed_cell_thickness(info.added)*2 );
//            sed_cell_add( sed_cube_to_remove(prof) , info.added );
         }
         else
         {
            mass_added     = 0;
            mass_lost      = 0;
            info.bruun_a   = 0;
            info.bruun_m   = 0;
            info.bruun_h_b = 0;
            info.bruun_y_0 = 0;
            info.bruun_y_b = 0;
            info.z_0       = 0;
         }

         p_info.mass_added = mass_added;
         p_info.mass_lost  = mass_lost;
         mass_after = sed_cube_mass( prof );

         eh_message( "time step (days)                : %f" , sed_ocean_storm_duration(this_storm) );
         eh_message( "cross shore current (m/s)       : %f" , xshore_current );
         eh_message( "incoming wave height (m)        : %f" , sed_ocean_storm_wave_height( this_storm ) );
         eh_message( "closure depth (m)               : %f" , info.z_0 );
         eh_message( "Bruun a (-)                     : %f" , info.bruun_a   );
         eh_message( "Bruun m (-)                     : %f" , info.bruun_m   );
         eh_message( "Bruun closure depth (m)         : %f" , info.bruun_h_b );
         eh_message( "Bruun start (m)                 : %f" , info.bruun_y_0 );
         eh_message( "Bruun end (m)                   : %f" , info.bruun_y_b );
//         eh_message( "along shore sediment added (kg) : %f" , sed_cell_mass( info.added ) );
//         eh_message( "sediment lost (kg)              : %f" , sed_cell_mass( info.lost  ) );
         eh_message( "mass before (kg)                : %f" , mass_before );
         eh_message( "mass after (kg)                 : %f" , mass_after  );
         eh_message( "mass added (kg)                 : %f" , mass_added  );

         eh_message( "mass balance (kg)               : %f" , (mass_after-mass_added)-mass_before );

         sed_cell_destroy( info.added );
         sed_cell_destroy( info.lost );
         eh_free( info.dt );
         lost = NULL;
      }
      else
      {
         eh_message( "time step (days)                : %f" , sed_ocean_storm_duration(this_storm) );
         eh_message( "cross shore current (m/s)       : %f" , xshore_current );
         eh_message( "incoming wave height (m)        : %f" , sed_ocean_storm_wave_height( this_storm ) );
         eh_message( "along shore sediment added (kg) : %f" , 0. );
         eh_message( "total sediment moved (kg)       : %f" , 0. );
      }

      current_time += sed_ocean_storm_duration(this_storm)*S_YEARS_PER_DAY;
      sed_cube_set_age( prof , current_time );

   }

//   sed_cube_set_time_step( prof , dt );
   sed_cube_set_age( prof , end_time );

   sed_cell_destroy( along_shore_sediment );

   return p_info;
}

gboolean
is_worth_running( Sed_ocean_storm s )
{
   return sed_ocean_storm_wave_height( s ) > .1;
}

#define S_KEY_ALONG_SHORE_SEDIMENT_NO "Grain type of along shore sediment"
#define S_KEY_XSHORE_VEL              "Cross shore current"

gboolean
init_xshore( Eh_symbol_table tab , gpointer ptr )
{
   Xshore_t *data=(Xshore_t*)ptr;
   GError* err = NULL;

   if ( tab == NULL )
   {
      eh_input_val_destroy( data->xshore_current );
      data->initialized = FALSE;
      return TRUE;
   }

   data->sediment_type = eh_symbol_table_int_value( tab , S_KEY_ALONG_SHORE_SEDIMENT_NO );

   if ( (data->xshore_current = eh_symbol_table_input_value(tab,S_KEY_XSHORE_VEL ,&err)) == NULL )
   {
      fprintf( stderr , "Unable to read input values: %s" , err->message );
      eh_exit(-1);
   }


   return TRUE;
}

