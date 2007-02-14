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
#include "sed_sedflux.h"
#include "diffusion.h"

static char *help_msg[] =
{
"                                                                             ",
" diffusion [options] [parameters]                                            ",
"  diffuse a bathymetric profile.                                             ",
"                                                                             ",
" Options                                                                     ",
"  v=yes          : toggle verbose mode. [off]                                ",
"  help=yes       : display this help message. [off]                          ",
"                                                                             ",
" Parameters                                                                  ",
"  k=k_val        : use k_val as the diffusion coefficient at the surface of  ",
"                   the water. [1.0]                                          ",
"  d=depth        : use depth as the water depth that the diffusion coefficent",
"                   will be 1% of its value at the surface.  the diffusion    ",
"                   coefficient falls off exponentially with depth to simulate",
"                   the attenuation of wave energy with depth. [100.0]        ",
"  dt=time_step   : use time_step as the time step for the model. [1.0]       ",
"  sea=sea_level  : use sea_level as the elevation of sea level.  the         ",
"                   diffusion coefficient will fall off exponentially from    ",
"                   this elevation. [0.0]                                     ",
"  dx=h_res       : use h_res as the horizontal resolution for the model (in  ",
"                   meters). [1.0]                                            ",
"  len=length     : use length as the length (in km) of the basin that is     ",
"                   being being modeled. [100.]                               ",
"  t=thickness    : use thickness to be the thickness (in meters) of the      ",
"                   sediment that already exists in the basin. [10.]          ",
"  in=file        : the input bathymetry file.  this is the bathymetry that   ",
"                   will be diffused.                                         ",
"  sed=file       : file is used to describe the sediment that will be        ",
"                   diffused.                                                 ",
"                                                                             ",
NULL
};

#define DEFAULT_K             (1.0)
#define DEFAULT_SKIN_DEPTH    (100.0)
#define DEFAULT_TIME_STEP     (1.)
#define DEFAULT_SEA_LEVEL     (0.)
#define DEFAULT_HORIZ_RES     (1.0)
#define DEFAULT_BASIN_LENGTH  (100.0)
#define DEFAULT_SED_THICKNESS (10.0)
#define DEFAULT_SEDIMENT_FILE_NAME "default.sediment"

int test_2d( void );

int main( int argc , char *argv[] )
{
   gssize i, j;
   gssize n_grains;
   char *bathy_file, *sediment_file;
   GArray *x, *bathymetry;
   Sed_sediment sediment_type;
   char *req[] = { "in" , NULL };
   double basin_length, horiz_res, sed_thickness;
   double k, skin_depth, time_step, sea_level;
   gboolean verbose;
   char *next_file;
   Eh_file_list *file_name;
   double *grain_fractions;
   double val;
   Sed_cell cell;
   Sed_cube prof;
   Sed_property_file sed_fp;
   Eh_args *args;
   Sed_property grain_size = sed_property_new( "grain" );
   GError* error = NULL;

   test_2d();

   args = eh_opts_init( argc , argv );
   if ( eh_check_opts( args , req , NULL , help_msg )!=0 )
      eh_exit(-1);

   k             = eh_get_opt_dbl ( args , "k"   , DEFAULT_K             );
   skin_depth    = eh_get_opt_dbl ( args , "d"   , DEFAULT_SKIN_DEPTH    );
   time_step     = eh_get_opt_dbl ( args , "dt"  , DEFAULT_TIME_STEP     );
   sea_level     = eh_get_opt_dbl ( args , "sea" , DEFAULT_SEA_LEVEL     );
   horiz_res     = eh_get_opt_dbl ( args , "dx"  , DEFAULT_HORIZ_RES     );
   basin_length  = eh_get_opt_dbl ( args , "len" , DEFAULT_BASIN_LENGTH  );
   sed_thickness = eh_get_opt_dbl ( args , "t"   , DEFAULT_SED_THICKNESS );
   bathy_file    = eh_get_opt_str ( args , "in"  , NULL                  );
   sediment_file = eh_get_opt_str ( args , "sed" , DEFAULT_SEDIMENT_FILE_NAME );
   verbose       = eh_get_opt_bool( args , "v"   , FALSE                 );

   x = g_array_new( FALSE , FALSE , sizeof(double) );

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
      eh_error( "%s: Unable to read sediment file: %s" , sediment_file , error->message);
   else
      sed_sediment_set_env( sediment_type );

   n_grains = sed_sediment_n_types( sediment_type );

   // initialize the sediment profile.
   prof = sed_cube_new( 1 , x->len );
   for (i=0;i<x->len;i++)
   {
      sed_column_set_base_height( sed_cube_col(prof,i) ,
                                    g_array_index( bathymetry , double , i )
                                  - sed_thickness );
      sed_column_set_y_position( sed_cube_col(prof,i) ,
                                 g_array_index( x , double , i ) );
   }
   sed_cube_set_y_res    ( prof , horiz_res );
   sed_cube_set_z_res    ( prof , sed_thickness/10 );
   sed_cube_set_sea_level( prof , sea_level );

   // add the existing sediment.
   grain_fractions = eh_new( double , n_grains );
   for ( i=0 ; i<n_grains ; i++ )
      grain_fractions[i] = 1./n_grains*sed_thickness;
   cell = sed_cell_new( n_grains );
   sed_cell_add_amount( cell , grain_fractions );
   for ( i=0 ; i<sed_cube_n_y(prof) ; i++ )
      sed_column_add_cell( sed_cube_col(prof,i) , cell );
   sed_cell_destroy( cell );
   eh_free( grain_fractions );

   file_name = eh_create_file_list( "output#.grain" );

for ( j=0 ; j<50 ; j++ )
{
   // diffuse the bathymetry.
   diffuse_sediment( prof , k , skin_depth , time_step , DIFFUSION_OPT_WATER );

   // write out the diffused bathymetry.
   for (i=0;i<sed_cube_n_y(prof);i++)
      fprintf( stdout , "%f , %f\n" ,
               sed_cube_col_y( prof , i ) ,
               sed_cube_water_depth( prof , 0 , i ) );

   next_file = eh_get_next_file( file_name );

   // write out the profile
   sed_fp=sed_property_file_new( next_file , grain_size , NULL );
   sed_property_file_write( sed_fp , prof );
   sed_property_file_destroy( sed_fp );
}

   sed_property_destroy( grain_size );
   g_array_free( x , FALSE );

   return 0;
}

int test_2d( void )
{
   gssize i, j;
   gssize n_grains;
   double *f;
   Sed_sediment sed;
   Sed_cell cell;
   Sed_cube cube;
   gssize n_x = 10;
   gssize n_y = 10;
   GError* error = NULL;

   sed = sed_sediment_scan( NULL , &error );
   if ( !sed )
      eh_error( "(null): Unable to read sediment file: %s" , error->message);

   n_grains = sed_sediment_n_types( sed );
   sed_sediment_set_env( sed );

   f    = eh_new( double , n_grains );
   cell = sed_cell_new( n_grains );
   cube = sed_cube_new( n_x , n_y );

   sed_cell_set_equal_fraction( cell );

   sed_cell_resize( cell , 50 );

   sed_cube_set_x_res( cube , 100 );
   sed_cube_set_y_res( cube , 100 );

   for ( i=0 ; i<n_x ; i++ )
      for ( j=0 ; j<n_y ; j++ )
         sed_cube_set_base_height( cube , i , j , -100 );

   sed_column_add_cell( sed_cube_col_ij(cube,n_x/2,n_y/2) , cell );
   
   diffuse_sediment_2( cube , 10 , 10 , 1000 , 365*50 , DIFFUSION_OPT_WATER );

   for ( i=0 ; i<n_x ; i++ )
   {
      for ( j=0 ; j<n_y ; j++ )
         fprintf( stdout , "%f " , sed_cube_water_depth( cube , i , j ) );
      fprintf( stdout , "\n" );
   }

   eh_free( f );
   sed_cell_destroy( cell );
   sed_sediment_destroy( sed );
   sed_sediment_unset_env( );
   sed_cube_destroy( cube );

   return 0;
}

