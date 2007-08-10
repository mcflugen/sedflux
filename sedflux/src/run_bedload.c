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

#define SED_BEDLOAD_PROC_NAME "bedload dumping"
#define EH_LOG_DOMAIN SED_BEDLOAD_PROC_NAME

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "my_processes.h"

#define BED_LOAD_SPREADING_ANGLE (14.*S_RADS_PER_DEGREE)

typedef struct
{
   double x_0;
   double y_0;
   double dx;
   double dy;
   double r_max;
   double min_angle;
   double max_angle;
}
Bed_load_data;

gboolean bed_load_2d_domain( double x , double y , Bed_load_data *user_data );
gboolean bed_load_1d_domain( double x , double y , Bed_load_data *user_data );

Sed_process_info
run_bedload( Sed_process p , Sed_cube prof )
{
   Bedload_dump_t*  data = sed_process_user_data(p);
   Sed_process_info info = SED_EMPTY_INFO;
   gssize i, j;
   gssize bed_load_n_x, bed_load_n_y;
   gssize low_x, low_y, high_x, high_y;
   double area, volume, thickness;
   double input_mass, add_mass;
   double bed_load_spreading_angle = BED_LOAD_SPREADING_ANGLE;
   Sed_cell bed_load_cell;
   Eh_dbl_grid fraction_grid;
   Eh_pt_2 *river_mouth;
   Sed_riv this_river;
   Sed_cell_grid in_suspension;
   Bed_load_data bed_load_data;

   this_river    = sed_cube_river_by_name( prof , data->river_name );
   in_suspension = sed_cube_in_suspension( prof , this_river );

   if ( sed_mode_is_3d() )
      river_mouth = sed_cube_river_mouth_position( prof , this_river );
   else
   {
      river_mouth    = eh_new( Eh_pt_2 , 1 );
      river_mouth->x = sed_cube_col_x(prof,0);
      river_mouth->y = sed_cube_col_y(prof,sed_cube_river_mouth_1d( prof ) );
   }

   if ( river_mouth )
   {
      river_mouth->x /= sed_cube_x_res( prof );
      river_mouth->y /= sed_cube_y_res( prof );
      river_mouth->x -= sed_river_mouth(this_river).i;
      river_mouth->y -= sed_river_mouth(this_river).j;
   }
   else
   {
      return SED_EMPTY_INFO;
   }

   bed_load_n_x = data->bed_load_dump_length / sed_cube_x_res( prof ) + 1;
   bed_load_n_y = data->bed_load_dump_length / sed_cube_y_res( prof ) + 1;

   eh_upper_bound( bed_load_n_x , sed_cube_n_x(prof) );
   eh_upper_bound( bed_load_n_y , sed_cube_n_y(prof) );

   bed_load_data.x_0       = river_mouth->x;
   bed_load_data.y_0       = river_mouth->y;
   bed_load_data.dx        = sed_cube_x_res( prof );
   bed_load_data.dy        = sed_cube_y_res( prof );
   bed_load_data.r_max     = data->bed_load_dump_length;
   bed_load_data.min_angle = sed_river_angle( this_river )
                           - bed_load_spreading_angle;
   bed_load_data.max_angle = sed_river_angle( this_river )
                           + bed_load_spreading_angle;
   //---
   // Create a grid to hold the bed load.  The river mouth will be at i=0,
   // j=0, and will point in the j-direction.
   //---
   fraction_grid = eh_grid_new( double , 2.*bed_load_n_x , 2.*bed_load_n_y );
   fraction_grid = eh_grid_reindex( fraction_grid ,
                                    -bed_load_n_x ,
                                    -bed_load_n_y );

   //---
   // Add the bed load into its grid.  The bed load is distributed evenly over
   // an arc.  The user defines the radius and interior angle of the arc.
   //---
   if ( sed_mode_is_3d() )
      eh_dbl_grid_populate( fraction_grid                      ,
                            (Populate_func)&bed_load_2d_domain ,
                            &bed_load_data );
   else
      eh_dbl_grid_populate( fraction_grid                      ,
                            (Populate_func)&bed_load_1d_domain ,
                            &bed_load_data );

   low_x  = eh_grid_low_x(fraction_grid);
   low_y  = eh_grid_low_y(fraction_grid);
   high_x = low_x + eh_grid_n_x(fraction_grid);
   high_y = low_y + eh_grid_n_y(fraction_grid);
   for ( i=low_x ; i<high_x ; i++ )
      for ( j=low_y ; j<high_y ; j++ )
      {
         if ( eh_dbl_grid_val(fraction_grid,i,j) > 0 )
            eh_dbl_grid_set_val( fraction_grid , i , j , 1. );
      }

   {
      double* just_bedload = eh_new0( double , sed_sediment_env_n_types() );

      just_bedload[0] = 1.;

      bed_load_cell = sed_cell_new_env( );

      sed_cell_set_fraction( bed_load_cell , just_bedload                        );
      sed_cell_set_facies  ( bed_load_cell , S_FACIES_BEDLOAD                    );
      sed_cell_set_age     ( bed_load_cell ,   sed_cube_age_in_years( prof )
                                             - sed_cube_time_step( prof )    );

      eh_free( just_bedload );
   }

   area      = eh_dbl_grid_sum( fraction_grid )
             * sed_cube_x_res( prof )
             * sed_cube_y_res( prof );
   volume    = (sed_river_bedload(this_river)*S_SECONDS_PER_DAY)
             * sed_cube_time_step_in_days( prof )
             / sed_cell_density_0( bed_load_cell );
   thickness = volume / area;

   if ( sed_mode_is_2d() && data->bed_load_dump_length > sed_cube_x_res(prof) )
      thickness *= sed_cube_x_res(prof)/data->bed_load_dump_length;

   low_x  = eh_grid_low_x(fraction_grid);
   low_y  = eh_grid_low_y(fraction_grid);
   high_x = low_x + eh_grid_n_x(fraction_grid);
   high_y = low_y + eh_grid_n_y(fraction_grid);

   //---
   // Add the bed load to the cube's in-suspension grid to be deposited later.
   //---
   for ( i=low_x ; i<high_x ; i++ )
      for ( j=low_y ; j<high_y ; j++ )
      {
         if( eh_dbl_grid_val(fraction_grid,i,j) > 0 )
         {
            sed_cell_resize( bed_load_cell ,
                             thickness*eh_dbl_grid_val(fraction_grid,i,j) );
            sed_cell_add( sed_cell_grid_val(in_suspension,i,j) ,
                          bed_load_cell );
         }
      }

   info.mass_added = sed_river_bedload( this_river )
                   * sed_cube_time_step_in_seconds( prof );
   info.mass_lost  = 0.;

   input_mass = sed_river_bedload( this_river )
              * sed_cube_time_step_in_seconds( prof );
   add_mass   = area*thickness*sed_cell_density_0( bed_load_cell );

   eh_message( "bedload dump thickness (m): %f" , thickness );
   eh_message( "bedload input (kg): %g" , input_mass );
   eh_message( "bedload added (kg): %g" , add_mass );

   eh_grid_destroy ( fraction_grid , TRUE );
   sed_cell_destroy( bed_load_cell );
   eh_free         ( river_mouth );

   return info;
}

#define BEDLOAD_KEY_DUMP_LEN   "distance to dump bedload"
#define BEDLOAD_KEY_RATIO      "ratio of flood plain to bedload rate"
#define BEDLOAD_KEY_RIVER_NAME "river name"

static gchar* bedload_req_labels[] =
{
   BEDLOAD_KEY_DUMP_LEN   ,
   BEDLOAD_KEY_RATIO      ,
   BEDLOAD_KEY_RIVER_NAME ,
   NULL
};

gboolean
init_bedload( Sed_process p , Eh_symbol_table t , GError** error )
{
   Bedload_dump_t* data    = sed_process_new_user_data( p , Bedload_dump_t );
   GError*         tmp_err = NULL;
   gboolean        is_ok   = TRUE;
   gchar**         err_s   = NULL;

   if ( eh_symbol_table_require_labels( t , bedload_req_labels , &tmp_err ) )
   {
      data->bed_load_dump_length = eh_symbol_table_dbl_value( t , BEDLOAD_KEY_DUMP_LEN   );
      data->bedload_ratio        = eh_symbol_table_dbl_value( t , BEDLOAD_KEY_RATIO      );
      data->river_name           = eh_symbol_table_value    ( t , BEDLOAD_KEY_RIVER_NAME );

      eh_check_to_s( data->bed_load_dump_length>0 , "Dump length positive"   , &err_s );
      eh_check_to_s( data->bedload_ratio>=0       , "Bedload ratio positive" , &err_s );

      if ( !tmp_err && err_s )
         eh_set_error_strv( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_PARAM , err_s );
   }

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
destroy_bedload( Sed_process p )
{
   if ( p )
   {
      Bedload_dump_t* data = sed_process_user_data( p );

      if ( data )
      {
         eh_free( data->river_name );
         eh_free( data             );
      }
   }

   return TRUE;
}

gboolean bed_load_1d_domain( double x , double y , Bed_load_data *user_data )
{
   double y_0 = user_data->y_0;
   double dy  = user_data->dy;
   double r_max = user_data->r_max;

   y*=dy;

   if ( y >= y_0 && y <= y_0+r_max && x>0 )
      return TRUE;
   
   return FALSE;
}

gboolean bed_load_2d_domain( double x , double y , Bed_load_data *user_data )
{
   double x_0 = user_data->x_0;
   double y_0 = user_data->y_0;
   double dx  = user_data->dx;
   double dy  = user_data->dy;
   double r_max = user_data->r_max;
   double a_min = user_data->min_angle;
   double a_max = user_data->max_angle;
   double r, a;

   r = sqrt( pow( (x - x_0)*dx , 2. ) + pow( (y - y_0)*dy , 2. ) );

   if ( r < r_max )
   {
      a_min = eh_reduce_angle( a_min );
      a_max = eh_reduce_angle( a_max );

      a = atan2( (y - y_0)*dy , (x - x_0)*dx );
      if ( a_min > a_max )
      {
         if ( a < a_max )
            a += 2.*M_PI;
         a_max += 2.*M_PI;
      }
      if ( a > a_min && a < a_max )
         return TRUE;
   }
   return FALSE;
}

