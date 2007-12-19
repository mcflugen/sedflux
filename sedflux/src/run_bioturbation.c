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

#define SED_BIOTURBATION_PROC_NAME "bioturbation"
#define EH_LOG_DOMAIN SED_BIOTURBATION_PROC_NAME

#include <stdio.h>
#include <sed/sed_sedflux.h>
#include "my_processes.h"
#include <bio.h>

Sed_process_info
run_bioturbation( Sed_process proc , Sed_cube p )
{
   Bioturbation_t*  data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;

   {
      gint   i;
      gint   len   = sed_cube_size                ( p );
      double dt    = sed_cube_time_step_in_seconds( p );
      double time  = sed_cube_age_in_years        ( p );
      double depth = eh_input_val_eval( data->depth , time );
      double k     = eh_input_val_eval( data->k     , time );

      for ( i=0 ; i<len ; i++ ) sed_column_bioturbate( sed_cube_col(p,i) , depth , k , dt );
   }

   return info;
}

#define BIO_KEY_DEPTH   "depth of bioturbation"
#define BIO_KEY_K       "bioturbation diffusion coefficient"

static gchar* bio_req_labels[] =
{
   BIO_KEY_DEPTH ,
   BIO_KEY_K     ,
   NULL
};

gboolean
init_bioturbation( Sed_process p , Eh_symbol_table t , GError** error )
{
   Bioturbation_t* data    = sed_process_new_user_data( p , Bioturbation_t );
   GError*         tmp_err = NULL;
   gboolean        is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );
   eh_require( t );
   eh_require( p );

   if ( eh_symbol_table_require_labels( t , bio_req_labels , &tmp_err ) )
   {
      data->k     = eh_symbol_table_input_value( t , BIO_KEY_K     , &tmp_err );
      data->depth = eh_symbol_table_input_value( t , BIO_KEY_DEPTH , &tmp_err );
   }

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
destroy_bioturbation( Sed_process p )
{
   if ( p )
   {
      Bioturbation_t* data = sed_process_user_data( p );

      if ( data )
      {
         eh_input_val_destroy( data->k     );
         eh_input_val_destroy( data->depth );
         eh_free             ( data        );
      }
   }
   return TRUE;
}

