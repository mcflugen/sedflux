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
#include "muds.h"

static const char *help_msg[] =
{
"                                                                             ",
" mud [options] [parameters]                                                  ",
"  move sediment in the bottom boundary layer.                                ",
"                                                                             ",
" Options                                                                     ",
"  v=yes          : toggle verbose mode. [off]                                ",
"  help=yes       : display this help message. [off]                          ",
"                                                                             ",
" Parameters                                                                  ",
"  waveh=height   : use height as the height of the waves acting on the       ",
"                   profile. [1]                                              ",
"  wavet=period   : use period as the period of the waves acting on the       ",
"                   profile. [15]                                             ",
"  sea=sea_level  : use sea_level as the elevation of sea level.  the         ",
"                   diffusion coefficient will fall off exponentially from    ",
"                   this elevation. [0.0]                                     ",
"  dx=h_res       : use h_res as the horizontal resolution for the model (in  ",
"                   meters). [1.0]                                            ",
"  len=length     : use length as the length (in m) of the basin that is      ",
"                   being being modeled. [100000.]                            ",
"  t=thickness    : use thickness to be the thickness (in meters) of the      ",
"                   sediment that already exists in the basin. [10.]          ",
"  in=file        : the input bathymetry file.  this is the bathymetry that   ",
"                   will be diffused.                                         ",
"  sed=file       : file is used to describe the sediment that will be        ",
"                   diffused.                                                 ",
"                                                                             ",
NULL
};

#define DEFAULT_WAVE_HEIGHT   (1.)
#define DEFAULT_WAVE_PERIOD   (15.)
#define DEFAULT_WAVE_LENGTH   (350.)
#define DEFAULT_SEA_LEVEL     (0.)
#define DEFAULT_HORIZ_RES     (1.0)
#define DEFAULT_BASIN_LENGTH  (100000.0)
#define DEFAULT_SED_THICKNESS (10.0)
#define DEFAULT_SED_FILE_NAME "default.sediment"

int main( int argc , char *argv[] )
{
   gssize i, j;
   gssize n_grains;
   gssize river_mouth;
   char *bathy_file, *sediment_file;
   GArray *x, *bathymetry;
   Sed_sediment sediment_type;
   char *req[] = { "in" , NULL };
   double basin_length, horiz_res, sed_thickness;
   double wave_height, wave_period, sea_level;
   double a, thickness;
   double mass_before, mass_after;
   gboolean verbose;
   double *grain_fractions, *wave;
   double val;
   Eh_file_list *file_name;
   char *next_file;
   Sed_cell cell;
   Sed_cell *deposit;
   Sed_cube prof;
   Sed_property_file sed_fp;
   Sed_property grain_size;
   Eh_args *args;
   GError* error = NULL;

   args = eh_opts_init( argc , argv );
   if ( eh_check_opts( args , req , NULL , help_msg )!=0 )
      eh_exit(-1);

   wave_height   = eh_get_opt_dbl ( args , "waveh" , DEFAULT_WAVE_HEIGHT   );
   wave_period   = eh_get_opt_dbl ( args , "wavet" , DEFAULT_WAVE_PERIOD   );
   sea_level     = eh_get_opt_dbl ( args , "sea"   , DEFAULT_SEA_LEVEL     );
   horiz_res     = eh_get_opt_dbl ( args , "dx"    , DEFAULT_HORIZ_RES     );
   basin_length  = eh_get_opt_dbl ( args , "len"   , DEFAULT_BASIN_LENGTH  );
   sed_thickness = eh_get_opt_dbl ( args , "t"     , DEFAULT_SED_THICKNESS );
   bathy_file    = eh_get_opt_str ( args , "in"    , NULL                  );
   sediment_file = eh_get_opt_str ( args , "sed"   , DEFAULT_SED_FILE_NAME );
   verbose       = eh_get_opt_bool( args , "v"     , FALSE                 );

   x = g_array_new( FALSE , FALSE , sizeof(double) );

   wave = eh_new( double , 3 );
   wave[0] = wave_height;
   wave[1] = wave_period;
   wave[2] = 5.*sed_gravity()*pow(wave_period*sinh(M_PI/10.)/M_PI,2.);

   if ( verbose )
   {
      fprintf( stderr , "wave height : %f\n" , wave[0] );
      fprintf( stderr , "wave period : %f\n" , wave[1] );
      fprintf( stderr , "wave length : %f\n" , wave[2] );
   }

   // set the positions where the sediment columns will be located.
   for ( val=0 ; val<basin_length ; val+=horiz_res )
      g_array_append_val(x,val);

   // read in the bathymetry.
   bathymetry = sed_get_floor( bathy_file , x , &error );
   if ( !bathymetry )
      eh_error( "%s: Unable to read bathymetry file: %s" , bathy_file , error->message );

   // read in the type of sediment.
   sediment_type = sed_sediment_scan( sediment_file , &error );
   if ( !sediment_type )
      eh_error( "%s: Unable to scan sediment file: %s" , sediment_file , error->message );
   n_grains = sed_sediment_n_types( sediment_type );
   sed_sediment_set_env( sediment_type );

   // initialize the sediment cube.
   prof = sed_cube_new( 1 , x->len );
   for (i=0;i<x->len;i++)
   {
      sed_column_set_base_height( sed_cube_col(prof,i) ,
                                    g_array_index( bathymetry , double , i )
                                  - sed_thickness );
      sed_column_set_y_position( sed_cube_col(prof,i) ,
                                 g_array_index( x , double , i ) );
   }
   sed_cube_set_x_res( prof , 1. );
   sed_cube_set_y_res( prof , horiz_res );
   sed_cube_set_z_res( prof , .1 );
   sed_cube_set_sea_level( prof , sea_level );

   file_name = eh_create_file_list( "output#.grain" );
   grain_size = sed_property_new( "grain" );

   for ( j=0 ; j<10 ; j++ )
   {
if ( j<75 )
   wave[0] *= 1.02;
else if ( j<150 )
   wave[0] *= .98;
else if ( j<225 )
   wave[0] *= 1.02;
else
   wave[0] *= .98;

//wave[2] = eh_get_fuzzy_dbl_norm( 200 , 20 );

      // add the existing sediment.
      grain_fractions = eh_new( double , n_grains );
      for ( i=0 ; i<n_grains ; i++ )
         grain_fractions[i] = 1./n_grains*sed_thickness;
   
      deposit = sed_cell_list_new( sed_cube_n_y(prof) , n_grains );
   
      cell = sed_cell_new( n_grains );
      sed_cell_add_amount( cell , grain_fractions );
      mass_before = sed_cube_mass( prof );
      river_mouth = sed_cube_river_mouth_1d( prof );
      for (i=river_mouth;i<sed_cube_n_y(prof);i++)
      {
         a = .001;
         thickness = sed_thickness*exp(-a*(  sed_cube_col_y(prof,i)
                                           - sed_cube_col_y(prof,river_mouth) ) );
         sed_cell_resize( cell , thickness );
         sed_cell_copy( deposit[i] , cell );
         mass_before +=   sed_cell_mass( deposit[i] )
                        * sed_cube_y_res( prof )*sed_cube_x_res( prof );
      }
      sed_cell_destroy( cell );
      eh_free( grain_fractions );

      // diffuse the bathymetry.
      muddy( prof , deposit , wave , 86400. );

      mass_after = sed_cube_mass( prof );

eh_watch_dbl( mass_before );
eh_watch_dbl( mass_after );

      if ( fabs(mass_after-mass_before)/mass_after > 1e-6 )
      {
         fprintf( stderr , "error : mass balance error\n" );
      }

      sed_cell_list_destroy( deposit );

      if ( j%2 == 0 )
      {
         // write out the cube
         next_file = eh_get_next_file( file_name );
         sed_fp=sed_property_file_new( next_file , grain_size , NULL );
         sed_property_file_write( sed_fp , prof );
         sed_property_file_destroy( sed_fp );
      }

   }

   // write out the diffused bathymetry.
   for (i=0;i<sed_cube_n_y(prof);i++)
      fprintf( stdout , "%f , %f\n" ,
               sed_cube_col_y( prof,i ) ,
               sed_column_thickness( sed_cube_col(prof,i) ) );

   // write out the cube
   sed_fp=sed_property_file_new( "output.grain" , grain_size , NULL );
   sed_property_file_write( sed_fp , prof );
   sed_property_file_destroy( sed_fp );

   sed_property_destroy( grain_size );

   g_array_free( x , FALSE );
   eh_free( wave );

   return 0;
}

