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

#include "plumevars.h"
#include "plumeinput.h"
#include "plume_approx.h"

#include "glib.h"
#include "utils.h"
#include "sed_sedflux.h"

Eh_opt_entry all_entries[] = {
   { "infile"  , 'i' , "Hydrotrend input file" , NULL , "-"                   },
   { "sedfile" , 's' , "Sediment input file"   , NULL , SED_SEDIMENT_TEST_FILE },
   { "len"     , 'n' , "Number of grid points" , NULL , "1000" },
   { "spacing" , 'd' , "Spacing between grid nodes" , NULL , "100" },
   { "width"   , 'w' , "Width of the basin"         , NULL , "1000" },
   { "verbose" , 'v' , "Verbose" , NULL , "TRUE" },
   { NULL }
};

int main(int argc, char *argv[])
{
   char* river_file;
   char* sediment_file;
   gssize len;
   double dx;
   double width;
   gboolean verbose;
   Sed_cell_grid dep;

   eh_init_glib();

   {
      Eh_opt_context this_context = eh_opt_create_context( "PLUME" ,
                                                           "Run plumes from a file"  ,
                                                           "Option specific to PLUME" );

      eh_opt_set_context  ( this_context , all_entries );
      eh_opt_parse_context( this_context , &argc , &argv , NULL );

      river_file    = eh_opt_value     ( this_context , "infile"  );
      sediment_file = eh_opt_value     ( this_context , "sedfile" );
      len           = eh_opt_int_value ( this_context , "len"     );
      dx            = eh_opt_dbl_value ( this_context , "spacing" );
      width         = eh_opt_dbl_value ( this_context , "width"   );
      verbose       = eh_opt_bool_value( this_context , "verbose" );
dx = 12.5;
   }

   {
      GError*      error    = NULL;
      Sed_sediment this_sed = sed_sediment_scan( sediment_file , &error );

      if ( !this_sed )
         eh_error( "%s: Unable to scan sediment file: %s" , sediment_file , error->message );

      sed_sediment_set_env( this_sed );
      sed_sediment_destroy( this_sed );
   }

   if ( verbose )
   {
      fprintf( stderr , "Number of grid nodes  : %d\n" , len );
      fprintf( stderr , "Spacing of grid nodes : %f\n" , dx );
      fprintf( stderr , "Basin width           : %f\n" , width );
      fprintf( stderr , "Input file            : %s\n" , river_file );
      fprintf( stderr , "Sediment file         : %s\n" , sediment_file );
   }

   {
      dep = eh_grid_new( Sed_cell , 1 , len );
      eh_grid_set_y_lin( dep , 0 , dx );
      sed_cell_grid_init( dep , sed_sediment_env_n_types() );
   }

   if ( river_file )
   {
      gssize i;
      double dt;
      Sed_hydro_file f = sed_hydro_file_new( river_file , SED_HYDRO_HYDROTREND , FALSE , TRUE , NULL );
//      Sed_hydro_file f = sed_hydro_file_new( river_file , HYDRO_HYDROTREND|HYDRO_USE_BUFFER , TRUE );
      Sed_hydro river;
      double mass_in  = 0;
      double mass_out = 0;

      for ( dt=0 ; dt<1 ; )
      {
         river = sed_hydro_file_read_record( f );

         mass_in += sed_hydro_suspended_load( river );

         dt += sed_hydro_duration( river );

         if ( verbose )
         {
            fprintf( stderr , "\r%.2f%%" , dt/365.*100. );
//            sed_hydro_fprint( stderr , river );
         }

         if ( dt <= 1 )
//            dep = plume_width_averaged_deposit( dep , river , sed_sediment_env() , width );
           dep = plume_width_averaged_deposit_num( dep , river , NULL , width );
//            dep = plume_centerline_deposit( dep , river , sed_sediment_env() );

         sed_hydro_destroy( river );

         for ( i=0 ; i<len-1 ; i++ )
            fprintf( stdout , "%.12f " , sed_cell_size( sed_cell_grid_val(dep,0,i) ) );
         fprintf( stdout , "%.12f\n" , sed_cell_size( sed_cell_grid_val(dep,0,i) ) );
      }

      if ( verbose )
         fprintf( stderr , "\n" );

      sed_hydro_file_destroy( f );

      for ( i=0 ; i<len ; i++ )
         mass_out += sed_cell_mass( sed_cell_grid_val(dep,0,i) )*dx*width ;

      if ( verbose )
      {
         fprintf( stderr , "Mass input from river (kg)   : %f\n" , mass_in  );
         fprintf( stderr , "Mass deposited by plume (kg) : %f\n" , mass_out );
      }

   }

   sed_cell_grid_free( dep );
   eh_grid_destroy( dep , TRUE );

   sed_sediment_unset_env();

   return 0;
}

