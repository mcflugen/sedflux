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

#include <stdio.h>
#include <sed/sed_sedflux.h>
#include "squall.h"

static char *help_msg[] =
{
"                                                                             ",
" squall [options] [parameters]                                               ",
"  simulate a storm hitting a shore.                                          ",
"                                                                             ",
" Options                                                                     ",
"  v=yes          : toggle verbose mode. [off]                                ",
"  help=yes       : display this help message. [off]                          ",
"                                                                             ",
" Parameters                                                                  ",
"  sea=sea_level  : use sea_level as the elevation of sea level.              ",
"  dx=h_res       : use h_res as the horizontal resolution for the model (in  ",
"                   meters). [1.0]                                            ",
"  t=thickness    : use thickness to be the thickness (in meters) of the      ",
"                   sediment that already exists in the basin. [10.]          ",
"  sed=file       : file is used to describe the sediment that will be        ",
"                   diffused.                                                 ",
"                                                                             ",
NULL
};

#define DEFAULT_SEA_LEVEL     (0.)
#define DEFAULT_HORIZ_RES     (1.)
#define DEFAULT_VERT_RES      (1.)
#define DEFAULT_DT            (1.)
#define DEFAULT_TT            (1.)
#define DEFAULT_WAVE_HEIGHT   (1.)
#define DEFAULT_WAVE_LENGTH   (10.)
#define DEFAULT_BASIN_LENGTH  (100.)
#define DEFAULT_OUTPUT_INT    (G_MAXINT)
#define DEFAULT_SED_THICKNESS (10.)
#define DEFAULT_SED_FILE      ("default.sediment")
#define DEFAULT_IN_FILE       ("default.bathy")


int main( int argc , char *argv[] )
{
   gssize i;
   gssize n_grains;
   char *bathy_file, *sed_file;
   GArray *x, *bathymetry;
   Sed_sediment sed;
   char *req[] = { "in" , NULL };
   double horiz_res, vert_res, dt, total_time;
   double basin_length, wave_height, wave_length;
   double sed_thickness, sea_level;
   int output_int;
   gboolean verbose;
   double t;
   Eh_file_list *file_name;
   double *grain_frac;
   double val;
   Sed_cell cell;
   Sed_cube p;
   Eh_args *args;
   GError* error = NULL;

   //---
   // parse the command line options.
   //---
   args = eh_opts_init( argc , argv );
   if ( eh_check_opts( args , req , NULL , help_msg )!=0 )
      eh_exit(-1);

   sea_level     = eh_get_opt_dbl ( args , "sea" , DEFAULT_SEA_LEVEL     );
   horiz_res     = eh_get_opt_dbl ( args , "dx"  , DEFAULT_HORIZ_RES     );
   vert_res      = eh_get_opt_dbl ( args , "dy"  , DEFAULT_VERT_RES      );
   dt            = eh_get_opt_dbl ( args , "dt"  , DEFAULT_DT            );
   total_time    = eh_get_opt_dbl ( args , "tt"  , DEFAULT_TT            );
   wave_height   = eh_get_opt_dbl ( args , "wh"  , DEFAULT_WAVE_HEIGHT   );
   wave_length   = eh_get_opt_dbl ( args , "wl"  , DEFAULT_WAVE_HEIGHT   );
   sed_thickness = eh_get_opt_dbl ( args , "t"   , DEFAULT_SED_THICKNESS );
   basin_length  = eh_get_opt_dbl ( args , "len" , DEFAULT_BASIN_LENGTH  );
   output_int    = eh_get_opt_int ( args , "oi"  , DEFAULT_OUTPUT_INT    );
   bathy_file    = eh_get_opt_str ( args , "in"  , DEFAULT_IN_FILE       );
   sed_file      = eh_get_opt_str ( args , "sed" , DEFAULT_SED_FILE      );
   verbose       = eh_get_opt_bool( args , "v"   , FALSE                 );

   if ( verbose )
      eh_print_all_opts( args , "squall" , stderr );

   //---
   // set the positions where the sediment columns will be located.
   //---
   x = g_array_new( FALSE , FALSE , sizeof(double) );
   for ( val=0 ; val<basin_length ; val+=horiz_res )
      g_array_append_val( x , val );

   //---
   // read in the bathymetry.
   //---
   bathymetry = sed_get_floor( bathy_file , x , &error );
   if ( !bathymetry )
      eh_error( "%s: Unable to read bathymetry file: %s" , bathy_file , error->message );

   //---
   // read in the sediment type from a file.
   //---
   sed = sed_sediment_scan( sed_file , &error );
   if ( !sed )
      eh_error( "%s: Unable to scan sediment file: %s" , sed_file , error->message );
   n_grains = sed_sediment_n_types( sed );

   //---
   // create a Sed_cube to hold the sediment.
   //---
   p = sed_cube_new( 1 , x->len );

   for ( i=0 ; i<x->len ; i++ )
   {
      sed_column_set_base_height( sed_cube_col(p,i) ,   g_array_index( bathymetry , double , i )
                                                      - sed_thickness );
      sed_column_set_y_position ( sed_cube_col(p,i) , g_array_index( x , double , i ) );
   }
   sed_cube_set_y_res( p , horiz_res );
   sed_cube_set_z_res( p , vert_res );
   sed_cube_set_x_res( p , 1 );
   sed_cube_set_sea_level( p , sea_level );
   sed_cube_set_wave_height( p , wave_height );
   sed_cube_set_wave_length( p , wave_length );
   sed_cube_set_time_step( p , dt );

   //---
   // add the existing sediment.
   //---
   grain_frac = eh_new( double , n_grains );
   for ( i=0 ; i<n_grains ; i++ )
      grain_frac[i] = 1./n_grains*sed_thickness;
   cell = sed_cell_new( n_grains );
   sed_cell_add_amount( cell , grain_frac );
   for ( i=0 ; i<sed_cube_n_y(p) ; i++ )
      sed_column_add_cell( sed_cube_col(p,i) , cell );
   sed_cell_destroy( cell );
   eh_free( grain_frac );

   file_name = eh_create_file_list( "output#.grain" );

   for ( t=0 , i=1 ; t<total_time ; t+=dt , i++ )
   {
      squall( p , dt );

      if ( i%output_int==0 )
         write_output_file( eh_get_next_file( file_name ) , p );
   }

   write_output_file( eh_get_next_file( file_name ) , p );

   g_array_free( x , FALSE );

   return 0;
}

void write_output_file( const char *file , Sed_cube p )
{
   Sed_property grain_size_in_phi = sed_property_new( "grain" );
   Sed_property_file sed_fp = sed_property_file_new( file ,
                                                     grain_size_in_phi ,
                                                     NULL );

   sed_property_file_write( sed_fp , p );
   sed_property_file_destroy( sed_fp );

   sed_property_destroy( grain_size_in_phi );

   return;
}

