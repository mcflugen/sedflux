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

#define SED_CONSTANTS_PROC_NAME "constants"
#define EH_LOG_DOMAIN SED_CONSTANTS_PROC_NAME

#include <stdio.h>
#include <math.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "my_processes.h"

Sed_process_info
run_constants( Sed_process proc , Sed_cube prof )
{
   Constants_t*     data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;

   {
      Sed_constants c;
      double cube_age = sed_cube_age_in_years( prof );

      c.gravity     = eh_input_val_eval( data->gravity     , cube_age );
      c.rho_sea_h2o = eh_input_val_eval( data->rho_sea_h2o , cube_age );
      c.rho_h2o     = eh_input_val_eval( data->rho_h2o     , cube_age );
      c.salinity    = eh_input_val_eval( data->salinity    , cube_age );
      c.rho_quartz  = eh_input_val_eval( data->rho_quartz  , cube_age );
      c.rho_mantle  = eh_input_val_eval( data->rho_mantle  , cube_age );

      set_cube_set_constants( prof , c );
   }

   sed_set_gravity        ( sed_cube_constants(prof).gravity     );
   sed_set_rho_sea_water  ( sed_cube_constants(prof).rho_sea_h2o );
   sed_set_rho_fresh_water( sed_cube_constants(prof).rho_h2o     );
   sed_set_sea_salinity   ( sed_cube_constants(prof).salinity    );
   sed_set_rho_quartz     ( sed_cube_constants(prof).rho_quartz  );
   sed_set_rho_mantle     ( sed_cube_constants(prof).rho_mantle  );

   eh_message( "time                 : %f" , sed_cube_age_in_years(prof)          );
   eh_message( "gravity              : %f" , sed_cube_constants(prof).gravity     );
   eh_message( "density of sea water : %f" , sed_cube_constants(prof).rho_sea_h2o );
   eh_message( "density of water     : %f" , sed_cube_constants(prof).rho_h2o     );
   eh_message( "density of quartz    : %f" , sed_cube_constants(prof).rho_quartz  );
   eh_message( "density of mantle    : %f" , sed_cube_constants(prof).rho_mantle  );

   return info;
}

#define S_KEY_CONST_GRAVITY     "acceleration due to gravity"
#define S_KEY_CONST_RHO_SEA_H2O "density of sea water"
#define S_KEY_CONST_RHO_H2O     "density of fresh water"
//#define S_KEY_CONST_MU_H2O      "kinematic viscosity of water"
//#define S_KEY_CONST_ETA_H2O     "dynamic viscosity of water"
#define S_KEY_CONST_SALINITY    "ocean salinity"
#define S_KEY_CONST_RHO_QUARTZ  "density of quartz"
#define S_KEY_CONST_RHO_MANTLE  "density of mantle"

gboolean
init_constants( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Constants_t* data    = sed_process_new_user_data( p , Constants_t );
   GError*     tmp_err = NULL;
   gboolean    is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   if ( !tmp_err ) data->gravity     = eh_symbol_table_input_value( tab , S_KEY_CONST_GRAVITY     , &tmp_err );
   if ( !tmp_err ) data->rho_sea_h2o = eh_symbol_table_input_value( tab , S_KEY_CONST_RHO_SEA_H2O , &tmp_err );
   if ( !tmp_err ) data->rho_h2o     = eh_symbol_table_input_value( tab , S_KEY_CONST_RHO_H2O     , &tmp_err );
   if ( !tmp_err ) data->salinity    = eh_symbol_table_input_value( tab , S_KEY_CONST_SALINITY    , &tmp_err );
   if ( !tmp_err ) data->rho_quartz  = eh_symbol_table_input_value( tab , S_KEY_CONST_RHO_QUARTZ  , &tmp_err );
   if ( !tmp_err ) data->rho_mantle  = eh_symbol_table_input_value( tab , S_KEY_CONST_RHO_MANTLE  , &tmp_err );

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
destroy_constants( Sed_process p )
{
   if ( p )
   {
      Constants_t* data = sed_process_user_data( p );

      if ( data )
      {
         eh_input_val_destroy( data->gravity     );
         eh_input_val_destroy( data->rho_sea_h2o );
         eh_input_val_destroy( data->rho_h2o     );
         eh_input_val_destroy( data->salinity    );
         eh_input_val_destroy( data->rho_quartz  );
         eh_input_val_destroy( data->rho_mantle  );
         eh_free             ( data              );
      }
   }

   return TRUE;
}
