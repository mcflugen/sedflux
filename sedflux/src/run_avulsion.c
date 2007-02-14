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
#include "run_avulsion.h"
#include "avulsion.h"
#include "processes.h"

Sed_process_info run_avulsion( gpointer ptr , Sed_cube prof )
{
   Avulsion_t*      data = (Avulsion_t*)ptr;
   Sed_process_info info = SED_EMPTY_INFO;
   Sed_riv          this_river;

   if ( prof == NULL )
   {
      if ( data->initialized )
      {
         g_rand_free( data->rand );
         data->rand        = NULL;
         data->reset_angle = TRUE;
         data->initialized = FALSE;
      }
      return info;
   }

   if ( !data->initialized )
   {
      sed_river_set_avulsion_data( sed_cube_river_by_name(prof,data->river_name) ,
                                   avulsion_new(NULL,0.) );
      data->rand = (data->rand_seed>0)?g_rand_new_with_seed( data->rand_seed ):g_rand_new();
      data->reset_angle = TRUE;
      data->initialized = TRUE;
   }

   this_river = sed_cube_river_by_name( prof , data->river_name );

   if ( this_river )
   {
      double time       = sed_cube_age_in_years( prof );
      double fraction   = eh_input_val_eval( data->f_remain  , time    );
      double std_dev    = eh_input_val_eval( data->std_dev   , time    )*S_RADS_PER_DEGREE;
      double min_angle  = eh_input_val_eval( data->min_angle , time    )*S_RADS_PER_DEGREE;
      double max_angle  = eh_input_val_eval( data->max_angle , time    )*S_RADS_PER_DEGREE;
      double f          = 10e6;

      if ( data->branching_is_on )
      {
         double area       = sed_cube_area_above( prof , sed_cube_sea_level(prof) );
         gint   n_branches = sed_cube_n_branches( prof );

         while ( area/n_branches > f )
         {
            sed_river_split( sed_river_longest_branch(this_river) );

            sed_river_impart_avulsion_data( this_river );

            n_branches += 2;
         }
      }

      sed_river_avulsion_data( this_river )->std_dev = std_dev;
      sed_river_avulsion_data( this_river )->rand    = data->rand;

      eh_require( max_angle>min_angle );
      eh_require( fraction>=0.        );
      eh_require( fraction<=1.        );
      eh_require( !eh_isnan(fraction) );

      if ( data->reset_angle )
      {
         data->reset_angle = FALSE;
         sed_river_set_angle( this_river , .5*(max_angle+min_angle) );
      }

      sed_river_set_angle_limit( this_river , min_angle     , max_angle     );
      sed_river_set_hinge      ( this_river , data->hinge_i , data->hinge_j );

      sed_cube_avulse_river( prof , this_river );
/*
      eh_message( "time         : %f" , sed_cube_age_in_years (prof)           );
      eh_message( "river name   : %s" , data->river_name                       );
      eh_message( "minimum angle: %f" , sed_river_min_angle   ( this_river )   );
      eh_message( "maximum angle: %f" , sed_river_max_angle   ( this_river )   );
      eh_message( "angle        : %f" , sed_river_angle_to_deg( this_river )   );
      eh_message( "position (x) : %d" , sed_river_mouth       ( this_river ).i );
      eh_message( "position (y) : %d" , sed_river_mouth       ( this_river ).j );
      eh_message( "fraction     : %f" , fraction                               );
      eh_message( "no. branches : %d" , sed_river_n_branches  ( this_river )   );
*/

      eh_data   ( "%f, %f, %f, %f, %d" ,
                  sed_cube_age_in_years ( prof       ) ,
                  sed_river_angle_to_deg( this_river ) ,
                  sed_river_angle_to_deg( sed_river_left (this_river) ) ,
                  sed_river_angle_to_deg( sed_river_right(this_river) ) ,
                  sed_river_n_branches  ( this_river ) );

      if ( sed_mode_is_2d() )
         sed_river_adjust_mass( this_river , fraction  );
   }
   else
      eh_require_not_reached();

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
/// If yes, the river is allowed to bifurcate
#define S_KEY_BRANCHING   "river can branch?"
/// Seed for the avulsion random number generator
#define S_KEY_SEED        "seed for random number generator"
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
      eh_exit( EXIT_FAILURE );
   }

   data->branching_is_on = eh_symbol_table_bool_value( tab , S_KEY_BRANCHING  );
   data->rand_seed       = eh_symbol_table_int_value ( tab , S_KEY_SEED       );
   data->river_name      = eh_symbol_table_value     ( tab , S_KEY_RIVER_NAME );

   hinge_point = g_strsplit ( eh_symbol_table_lookup( 
                                  tab ,
                                  S_KEY_HINGE_POINT ) ,
                              "," , -1 );

   if ( hinge_point[0] && hinge_point[1] )
   {
      data->hinge_i = strtoul( hinge_point[0] , NULL , 10 );
      data->hinge_j = strtoul( hinge_point[1] , NULL , 10 );
   }
   else
      eh_error( "An x-y pair is required for the hinge point." );

   if ( sed_mode_is_2d() )
   {
      data->hinge_i = 0;
      data->hinge_j = 0;
   }

   g_strfreev( hinge_point );

   data->reset_angle = TRUE;

   return TRUE;
}

