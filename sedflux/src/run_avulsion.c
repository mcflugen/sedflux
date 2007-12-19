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
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include <avulsion.h>
#include "my_processes.h"

#include "sedflux.h"

gboolean init_avulsion_data( Sed_process p , Sed_cube prof );

Sed_process_info
run_avulsion( Sed_process p , Sed_cube prof )
{
   Avulsion_t*      data = sed_process_user_data(p);
   Sed_process_info info = SED_EMPTY_INFO;
   Sed_riv          this_river;

   if ( sed_process_run_count(p)==0 )
      init_avulsion_data( p , prof );

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
            sed_cube_split_river( prof , sed_river_name_loc(this_river) );

            sed_river_impart_avulsion_data( this_river );

            n_branches += 2;
         }
      }

      eh_require( sed_river_avulsion_data( this_river ) );

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
/*
      eh_data   ( "%f, %f, %f, %f, %d" ,
                  sed_cube_age_in_years ( prof       ) ,
                  sed_river_angle_to_deg( this_river ) ,
                  sed_river_angle_to_deg( sed_river_left (this_river) ) ,
                  sed_river_angle_to_deg( sed_river_right(this_river) ) ,
                  sed_river_n_branches  ( this_river ) );
*/

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
#define AVULSION_KEY_STDDEV      "standard deviation"
/// Minumum angle that the river mouth can make with the hinge point
#define AVULSION_KEY_MIN_ANGLE   "minimum angle"
/// Maximum angle that the river mouth can make with the hinge point
#define AVULSION_KEY_MAX_ANGLE   "maximum angle"
/// The name of the river that this avulsion process is associated with
#define AVULSION_KEY_RIVER_NAME  "river name"
/// The (x,y) location of the hinge point
#define AVULSION_KEY_HINGE_POINT "hinge point"
/// Fraction of total sediment to be placed within this profile
/// (2D-sedflux only)
#define AVULSION_KEY_FRACTION    "fraction of sediment remaining in plane"
/// If yes, the river is allowed to bifurcate
#define AVULSION_KEY_BRANCHING   "river can branch?"
/// Seed for the avulsion random number generator
#define AVULSION_KEY_SEED        "seed for random number generator"
/* @} */

static gchar* avulsion_req_labels[] =
{
   AVULSION_KEY_STDDEV      ,
   AVULSION_KEY_MIN_ANGLE   ,
   AVULSION_KEY_MAX_ANGLE   ,
   AVULSION_KEY_RIVER_NAME  ,
   AVULSION_KEY_FRACTION    ,
   AVULSION_KEY_BRANCHING   ,
   AVULSION_KEY_SEED        ,
   NULL
};
static gchar* avulsion_3d_req_labels[] =
{
   AVULSION_KEY_HINGE_POINT ,
   NULL
};

gboolean
init_avulsion( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Avulsion_t* data    = sed_process_new_user_data( p , Avulsion_t );
   GError*     tmp_err = NULL;
   gboolean    is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   data->rand = NULL;

   if ( eh_symbol_table_require_labels( tab , avulsion_req_labels , &tmp_err ) )
   {
      if ( !tmp_err ) data->std_dev   = eh_symbol_table_input_value( tab , AVULSION_KEY_STDDEV    , &tmp_err );
      if ( !tmp_err ) data->f_remain  = eh_symbol_table_input_value( tab , AVULSION_KEY_FRACTION  , &tmp_err );
      if ( !tmp_err ) data->min_angle = eh_symbol_table_input_value( tab , AVULSION_KEY_MIN_ANGLE , &tmp_err );
      if ( !tmp_err ) data->max_angle = eh_symbol_table_input_value( tab , AVULSION_KEY_MAX_ANGLE , &tmp_err );

      data->branching_is_on = eh_symbol_table_bool_value( tab , AVULSION_KEY_BRANCHING  );
      data->rand_seed       = eh_symbol_table_int_value ( tab , AVULSION_KEY_SEED       );
      data->river_name      = eh_symbol_table_value     ( tab , AVULSION_KEY_RIVER_NAME );

      data->reset_angle     = TRUE;

      data->hinge_i = 0;
      data->hinge_j = 0;

      if (    sed_mode_is_3d()
           && eh_symbol_table_require_labels( tab , avulsion_3d_req_labels , &tmp_err ) )
      {
         gchar** err_s = NULL;
         gchar** hinge = g_strsplit( eh_symbol_table_lookup( tab , AVULSION_KEY_HINGE_POINT ) , "," , -1 );

         eh_check_to_s( hinge[0] && hinge[1] , "Need i and j index for hinge point" , &err_s );

         if ( hinge[0] && hinge[1] )
         {
            data->hinge_i = strtoul( hinge[0] , NULL , 10 );
            data->hinge_j = strtoul( hinge[1] , NULL , 10 );
         }

         g_strfreev( hinge );

         eh_check_to_s( data->hinge_i>=0 , "Hinge index positive integer" , &err_s );
         eh_check_to_s( data->hinge_j>=0 , "Hinge index positive integer" , &err_s );

         if ( !tmp_err && err_s )
            eh_set_error_strv( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_PARAM , err_s );
      }
   }

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
init_avulsion_data( Sed_process p , Sed_cube prof )
{
   Avulsion_t* data = sed_process_user_data( p );

   if ( data )
   {
      Sed_riv r = sed_cube_river_by_name( prof , data->river_name );
      
      sed_river_set_avulsion_data( r , avulsion_new(NULL,0.) );

      if ( data->rand_seed>0 ) data->rand = g_rand_new_with_seed( data->rand_seed );
      else                     data->rand = g_rand_new();

      data->reset_angle = TRUE;
   }

   return TRUE;
}

gboolean
destroy_avulsion( Sed_process p )
{
   if ( p )
   {
      Avulsion_t* data = sed_process_user_data( p );

      if ( data )
      {
         if ( data->rand ) g_rand_free( data->rand );

         eh_input_val_destroy( data->min_angle  );
         eh_input_val_destroy( data->max_angle  );
         eh_input_val_destroy( data->std_dev    );
         eh_input_val_destroy( data->f_remain   );
         eh_free             ( data->river_name );
         eh_free             ( data             );
      }
   }

   return TRUE;
}

