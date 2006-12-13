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

#define SED_AVULSION_PROC_NAME "avulsion"
#define EH_LOG_DOMAIN SED_AVULSION_PROC_NAME

#include <stdio.h>
#include <math.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "avulsion.h"
#include "processes.h"

Sed_process_info run_avulsion( gpointer ptr , Sed_cube prof )
{
   Avulsion_t *data=(Avulsion_t*)ptr;
   Sed_river *this_river;
   double fraction, angle;
   int n;
   Sed_process_info info = SED_EMPTY_INFO;

   if ( prof == NULL )
   {
      if ( data->initialized )
      {
         data->last_angle = 0.;
         data->initialized = FALSE;
      }
      return info;
   }

   if ( !data->initialized )
   {
      data->hinge->min_angle = eh_input_val_eval(
                                  data->min_angle ,
                                  sed_cube_age_in_years( prof ) );
      data->hinge->max_angle = eh_input_val_eval(
                                  data->max_angle ,
                                  sed_cube_age_in_years( prof ) );
      data->hinge->angle     = (   data->hinge->min_angle
                                 + data->hinge->max_angle )
                             / 2. * S_RADS_PER_DEGREE;
      data->last_angle       = data->hinge->angle;

      data->initialized = TRUE;
   }

   this_river = sed_cube_river_by_name( prof , data->river_name );

   if ( this_river )
   {
      data->hinge->std_dev   = eh_input_val_eval(
                                  data->std_dev ,
                                  sed_cube_age_in_years( prof ) );
      data->hinge->min_angle = eh_input_val_eval(
                                  data->min_angle ,
                                  sed_cube_age_in_years( prof ) );
      data->hinge->max_angle = eh_input_val_eval(
                                  data->max_angle ,
                                  sed_cube_age_in_years( prof ) );
      fraction               = eh_input_val_eval(
                                  data->f_remain ,
                                  sed_cube_age_in_years( prof ) );

      if ( eh_isnan( fraction ) )
      {
         fraction = 1.;
         eh_warning( "fraction is NaN.  Setting to 1." );
      }
      eh_clamp( fraction , 0. , 1. );

      this_river->hinge->std_dev   = data->hinge->std_dev;
      this_river->hinge->angle     = data->last_angle;
      this_river->hinge->min_angle = data->hinge->min_angle*S_RADS_PER_DEGREE;
      this_river->hinge->max_angle = data->hinge->max_angle*S_RADS_PER_DEGREE;
      this_river->hinge->x         = data->hinge->x;
      this_river->hinge->y         = data->hinge->y;

      sed_avulse_river( this_river , prof );

      angle                    = sed_get_river_angle( this_river );
      data->last_angle         = angle;
      this_river->hinge->angle = eh_reduce_angle( data->last_angle );

      eh_message( "time         : %f" , sed_cube_age_in_years(prof)       );
      eh_message( "river name   : %s" , data->river_name                  );
      eh_message( "minimum angle: %f" , data->hinge->min_angle            );
      eh_message( "maximum angle: %f" , data->hinge->max_angle            );
      eh_message( "angle        : %f" , sed_get_river_angle( this_river )
                                        / S_RADS_PER_DEGREE               );
      eh_message( "position (x) : %d" , this_river->x_ind                 );
      eh_message( "position (y) : %d" , this_river->y_ind                 );
      eh_message( "fraction     : %f" , fraction                          );

/*
      if ( !is_sedflux_3d() )
         angle -= G_PI/2.;

      fraction = exp( -angle*angle );

*/
      if ( !is_sedflux_3d() )
      {
         gssize n_grains = sed_hydro_size( this_river->data );
         Sed_hydro this_data = this_river->data;
         for ( n=0;n<n_grains-1;n++)
            sed_hydro_set_nth_concentration( this_data , n ,
                                             sed_hydro_nth_concentration(this_data,n)*fraction );
         sed_hydro_set_bedload( this_data , sed_hydro_bedload(this_data)*fraction);
      }

//      eh_message( "fraction     : %f" , fraction                          );
   }

   return info;
}

/** \name Input paramaters for avulsion module.
@{
*/
/// Standard deviation of angle change of river mouth with hinge point
#define S_KEY_STDDEV      "standard deviation"
/// Minumum angle that the river mouth can make with the hinge point
#define S_KEY_MIN_ANGLE   "minimum angle"
/// Maximum angle that the river mouth can make with the hinge point
#define S_KEY_MAX_ANGLE   "maximum angle"
/// The name of the river that this avulsion process is associated with
#define S_KEY_RIVER_NAME  "river name"
/// The (x,y) location of the hinge point
#define S_KEY_HINGE_POINT "hinge point"
/// Fraction of total sediment to be placed within this profile
/// (2D-sedflux only)
#define S_KEY_FRACTION    "fraction of sediment remaining in plane"
/* @} */

gboolean init_avulsion( Eh_symbol_table tab , gpointer ptr )
{
   Avulsion_t *data=(Avulsion_t*)ptr;
   char **hinge_point;
   GError* err = NULL;

   if ( tab == NULL )
   {
      eh_input_val_destroy( data->min_angle );
      eh_input_val_destroy( data->max_angle );
      eh_input_val_destroy( data->std_dev   );
      eh_input_val_destroy( data->f_remain  );
      eh_free( data->hinge );
      eh_free( data->river_name );
      data->initialized = FALSE;
      return TRUE;
   }

   if (    (data->std_dev   = eh_symbol_table_input_value(tab,S_KEY_STDDEV   ,&err)) == NULL
        || (data->f_remain  = eh_symbol_table_input_value(tab,S_KEY_FRACTION ,&err)) == NULL
        || (data->min_angle = eh_symbol_table_input_value(tab,S_KEY_MIN_ANGLE,&err)) == NULL
        || (data->max_angle = eh_symbol_table_input_value(tab,S_KEY_MAX_ANGLE,&err)) == NULL )
   {
      fprintf( stderr , "Unable to read input values: %s" , err->message );
      eh_exit(-1);
   }

   data->hinge = eh_new( Sed_hinge_pt , 1 );

   data->river_name = eh_symbol_table_value( tab , S_KEY_RIVER_NAME );

   hinge_point = g_strsplit ( eh_symbol_table_lookup( 
                                  tab ,
                                  S_KEY_HINGE_POINT ) ,
                              "," , -1 );

   if ( hinge_point[0] && hinge_point[1] )
   {
      data->hinge->x = strtoul( hinge_point[0] , NULL , 10 );
      data->hinge->y = strtoul( hinge_point[1] , NULL , 10 );
   }
   else
      eh_error( "An x-y pair is required for the hinge point." );

   if ( !is_sedflux_3d() )
   {
      data->hinge->x = 0;
      data->hinge->y = 0;
   }

   g_strfreev( hinge_point );

   return TRUE;
}

