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
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "my_processes.h"
#include "sedflux.h"

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

double   deposit_in_ocean  ( Sed_cube p , Sed_riv r , double vol , Eh_dbl_grid fraction_grid );
double   deposit_in_river  ( Sed_cube p , Sed_riv r , double vol );
gboolean bed_load_2d_domain( double x , double y , Bed_load_data *user_data );
gboolean bed_load_1d_domain( double x , double y , Bed_load_data *user_data );

Sed_process_info
run_bedload( Sed_process p , Sed_cube prof )
{
   Bedload_dump_t*  data       = sed_process_user_data(p);
   Sed_process_info info       = SED_EMPTY_INFO;
   Sed_riv          this_river = sed_cube_river_by_name( prof , data->river_name );

   if ( this_river )
   { /* If there is a river by this name. */
      Eh_pt_2*      river_mouth;

      /* The river mouth's position. */
      if ( sed_mode_is_3d() )
         river_mouth = sed_cube_river_mouth_position( prof , this_river );
      else
      {
         river_mouth    = eh_new( Eh_pt_2 , 1 );
         river_mouth->x = sed_cube_col_x(prof,0);
         river_mouth->y = sed_cube_col_y(prof,sed_cube_river_mouth_1d( prof ) );
      }

      /* If there is a river mouth. */
      if ( river_mouth )
      {
//         Sed_cell    bed_load_cell = sed_cell_new_env( );
         gint        bed_load_n_x  = data->bed_load_dump_length / sed_cube_x_res( prof ) + 1;
         gint        bed_load_n_y  = data->bed_load_dump_length / sed_cube_y_res( prof ) + 1;
         double      mass_ocean    = 0;
         double      mass_delta    = 0;
         Eh_dbl_grid fraction_grid;

         eh_upper_bound( bed_load_n_x , sed_cube_n_x(prof) );
         eh_upper_bound( bed_load_n_y , sed_cube_n_y(prof) );

         //---
         // Create a grid to hold the bed load.  The river mouth will be at i=0,
         // j=0, and will point in the j-direction.
         //---
         fraction_grid = eh_grid_new( double , 2.*bed_load_n_x , 2.*bed_load_n_y );

         if ( fraction_grid )
         { /* Determine what fraction of each cell is filled with sediment. */
            Bed_load_data bed_load_data;
            double bed_load_spreading_angle = BED_LOAD_SPREADING_ANGLE;

            fraction_grid = eh_grid_reindex( fraction_grid , -bed_load_n_x , -bed_load_n_y );

            river_mouth->x /= sed_cube_x_res( prof );
            river_mouth->y /= sed_cube_y_res( prof );
            river_mouth->x -= sed_river_mouth(this_river).i;
            river_mouth->y -= sed_river_mouth(this_river).j;

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
         }

         eh_debug( "set non-zero elements to 1" );
         { /* Set non-zero elements of fraction to 1. */
            gint i, j;
            const gint low_x  = eh_grid_low_x(fraction_grid);
            const gint low_y  = eh_grid_low_y(fraction_grid);
            const gint high_x = low_x + eh_grid_n_x(fraction_grid);
            const gint high_y = low_y + eh_grid_n_y(fraction_grid);

            /* Note: this should probably be changed so that some elements are only fractionally filled. */
            for ( i=low_x ; i<high_x ; i++ )
               for ( j=low_y ; j<high_y ; j++ )
               {
                  if ( eh_dbl_grid_val(fraction_grid,i,j) > 0 )
                     eh_dbl_grid_set_val( fraction_grid , i , j , 1. );
               }
         }

         eh_debug( "set cell's facies, age, type" );
         { /* Set the bedload cell's facies, age, and type */
/*
            bed_load_cell = sed_cell_new_bedload( NULL , 0 );

            sed_cell_set_facies  ( bed_load_cell , S_FACIES_BEDLOAD                    );
            sed_cell_set_age     ( bed_load_cell ,   sed_cube_age_in_years( prof )
                                                   - sed_cube_time_step( prof )    );
*/
         }

/*
         {
            double area      = eh_dbl_grid_sum( fraction_grid )
                             * sed_cube_x_res( prof )
                             * sed_cube_y_res( prof );
            double vol_total = (sed_river_bedload(this_river)*S_SECONDS_PER_DAY)
                             * sed_cube_time_step_in_days( prof )
                             / sed_cell_density_0( bed_load_cell );
            double vol_delta = vol_total * data->f_retained;

            thickness = (vol_tot-vol_delta) / area;

            if ( sed_mode_is_2d() && data->bed_load_dump_length > sed_cube_x_res(prof) )
               thickness *= sed_cube_x_res(prof)/data->bed_load_dump_length;

         }
*/

         eh_debug( "deposit sediment" );
         { /* Deposit the sediment landward and seaward of the river mouth. */
            double vol_total = (sed_river_bedload(this_river)*S_SECONDS_PER_DAY)
                             * sed_cube_time_step_in_days( prof )
                             / sed_type_rho_sat( sed_sediment_type( NULL , 0 ) );
/* / sed_cell_density_0( bed_load_cell ); */
            double vol_delta = vol_total*data->f_retained;
            double vol_ocean = vol_total*(1.-data->f_retained);

            if ( vol_ocean>0 ) mass_ocean = deposit_in_ocean( prof , this_river , vol_ocean , fraction_grid );
            if ( vol_delta>0 ) mass_delta = deposit_in_river( prof , this_river , vol_delta );
         }

         eh_debug( "mass balance" );
         { /* Mass balance */
            double input_mass = sed_river_bedload( this_river )
                              * sed_cube_time_step_in_seconds( prof );
            double add_mass   = mass_ocean + mass_delta;
            //double add_mass   = area*thickness*sed_type_rho_sat( sed_sediment_type( NULL , 0 ) );
            //double add_mass   = area*thickness*sed_cell_density_0( bed_load_cell );

            info.mass_added = sed_river_bedload( this_river )
                            * sed_cube_time_step_in_seconds( prof );
            info.mass_lost  = 0.;

            //eh_message( "bedload dump thickness (m): %f"       , thickness  );
            eh_message( "bedload input (kg): %g"               , input_mass   );
            eh_message( "bedload added (kg): %g"               , add_mass     );
            eh_message( "mass added to delta plain (kg): %g"   , mass_delta   );
            eh_message( "mass added to ocean (kg): %g"         , mass_ocean   );
         }

         eh_grid_destroy ( fraction_grid , TRUE );
//         sed_cell_destroy( bed_load_cell );
      }

      eh_free( river_mouth );
   }

   return info;
}

#define BEDLOAD_KEY_DUMP_LEN   "distance to dump bedload"
#define BEDLOAD_KEY_RATIO      "ratio of flood plain to bedload rate"
#define BEDLOAD_KEY_RETAINED   "fraction of bedload retained in the delta plain"
#define BEDLOAD_KEY_RIVER_NAME "river name"

static gchar* bedload_req_labels[] =
{
   BEDLOAD_KEY_DUMP_LEN   ,
   BEDLOAD_KEY_RATIO      ,
   BEDLOAD_KEY_RETAINED   ,
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
      data->f_retained           = eh_symbol_table_dbl_value( t , BEDLOAD_KEY_RETAINED   );
      data->river_name           = eh_symbol_table_value    ( t , BEDLOAD_KEY_RIVER_NAME );

      eh_check_to_s( data->bed_load_dump_length>0 , "Dump length positive"            , &err_s );
      eh_check_to_s( data->bedload_ratio>=0       , "Bedload ratio positive"          , &err_s );
      eh_check_to_s( data->f_retained>=0          , "Bedload retention positive"      , &err_s );
      eh_check_to_s( data->f_retained<=1.         , "Bedload retention less than one" , &err_s );

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

double
deposit_in_ocean( Sed_cube p , Sed_riv r , double vol , Eh_dbl_grid fraction_grid )
{
   double mass_dep = 0.;

   eh_require( p             );
   eh_require( r             );
   eh_require( fraction_grid );
   eh_require( vol>=0        );

   if ( vol>0 )
   { /* If there is sediment to deposit */
      Sed_cell_grid in_suspension = sed_cube_in_suspension( p , r );

      if ( in_suspension )
      {
         const double area = eh_dbl_grid_sum( fraction_grid )
                           * sed_cube_x_res( p )
                           * sed_cube_y_res( p );

         if ( area>0 )
         {  /* Add the bed load to the cube's in-suspension grid to be deposited later. */
            //Sed_cell c     = sed_cell_new_bedload( NULL , area/vol );
            Sed_cell c     = sed_cell_new_bedload( NULL , vol/area );

         { /* The thickness of sediment to be deposited. */
/*
            if ( sed_mode_is_2d() && data->bed_load_dump_length > sed_cube_x_res(p) )
               thickness *= sed_cube_x_res(p)/data->bed_load_dump_length;
*/
         }

         {
            gint   i, j;
            const gint   low_x  = eh_grid_low_x(fraction_grid);
            const gint   low_y  = eh_grid_low_y(fraction_grid);
            const gint   high_x = low_x + eh_grid_n_x(fraction_grid);
            const gint   high_y = low_y + eh_grid_n_y(fraction_grid);
            const double t      = vol / area;

            for ( i=low_x ; i<high_x ; i++ )
               for ( j=low_y ; j<high_y ; j++ )
               {
                  if( eh_dbl_grid_val(fraction_grid,i,j) > 0 )
                  {
                     sed_cell_resize( c , t*eh_dbl_grid_val(fraction_grid,i,j) );
                     mass_dep += sed_cell_mass( c );
                     sed_cell_add   ( sed_cell_grid_val(in_suspension,i,j) , c );
                  }
               }
            mass_dep *= sed_cube_x_res(p)*sed_cube_y_res(p);

            sed_cell_destroy( c );
         }
         }
      }
      else
         eh_require_not_reached();
   }

   return mass_dep;
}

/** Deposit sediment along the profile of a river.

@param p    A Sed_cube
@param r    A Sed_riv
@param vol  Volume of bedload to deposit along river (m^3)

@return The volume (m^3) of sediment deposited
*/
double
deposit_in_river( Sed_cube p , Sed_riv r , double vol )
{
   double mass_dep = 0;

   eh_require( p      );
   eh_require( r      );
   eh_require( vol>=0 );

   if ( p && r && vol>0 )
   {
      gint* river_path = sed_cube_river_path_id( p , r , TRUE );

      eh_require( river_path );

      if ( river_path )
      { /* If there is a river path. */
         Sed_cube  river_profile = sed_cube_cols( p , river_path );
         Sed_hydro river_data    = sed_river_hydro( r );
         gint      i_river       = sed_cube_river_mouth_1d( river_profile ) - 1;

         eh_require( river_data    );
         eh_require( river_profile );

         if ( i_river>0 )
         {  /* If the river has some length. */

            if ( sed_mode_is_3d() )
            {
               sed_cube_set_x_res( river_profile , sed_river_width(r) );
               sed_cube_set_y_res( river_profile , .5*(sed_cube_x_res(p)+sed_cube_y_res(p)) );
            }
            else
            {
               sed_cube_set_x_res( river_profile , sed_cube_x_res(p) );
               sed_cube_set_y_res( river_profile , sed_cube_y_res(p) );
            }
            { /* Deposit the sediment. */
               gint   i;
               double area    = sed_cube_x_res( river_profile )
                              * sed_cube_y_res( river_profile )
                              * i_river;
               Sed_cell c     = sed_cell_new_bedload( NULL , vol/area );
               //Sed_cell c     = sed_cell_new_bedload( NULL , area/vol );
               for ( i=0 ; i<i_river ; i++ )
               {
                  mass_dep += sed_cell_mass(c);
                  sed_column_add_cell( sed_cube_col(river_profile,i) , c );
               }
               mass_dep *= sed_cube_x_res( river_profile )*sed_cube_y_res( river_profile );
               //vol_dep = vol;

               sed_cell_destroy( c );
            }

         }

         eh_free          ( river_path );
         sed_cube_free    ( river_profile , FALSE );
      }
   }

   return mass_dep;
}

/*
Sed_cell
sed_cube_add_cell( Sed_cube c , Sed_cell c )
{
   Sed_cell eroded_sed = sed_cell_new_env();

   eh_require( river_profile );

   if (    river_profile 
        && sed_cube_river_mouth_1d( river_profile) > 0 )
   {
      gint     i_river = sed_cube_river_mouth_1d( river_profile ) - 1;

      Sed_cell eroded  = sed_cell_new_env( );
      double   dx      = sed_cube_y_res( river_profile );
      double   width   = sed_cube_x_res( river_profile );
      Erosion_lin_st linear_const = erosion_get_linear_constants( slope );
      gint i;
      double x;
      double height;
      double river_height;
      double erode_height;
         
      river_height = sed_cube_top_height( river_profile , 0 , i_river );
      
      for ( i=i_river-1 ; i>=0 ; i-- )
      {
         x      = (i-i_river)*dx;
         height = erosion_get_linear_height( x , linear_const ) + river_height;
         
         erode_height = sed_cube_top_height(river_profile,0,i)-height;
         if ( erode_height > 1e-12 )
         {
            sed_column_extract_top( sed_cube_col(river_profile,i) ,
                                    erode_height                 ,
                                    eroded);
            sed_cell_add( eroded_sed , eroded );
         }
      }
         
      // convert time step to seconds.
      //dt = sed_cube_time_step_in_seconds( river_profile );
         
            // add the eroded sediment to the river discharge.
   //         river_data = sed_cube_river_data( river_profile );
         
//      volume_eroded = sed_cell_size(eroded_sed)*dx*width;

      sed_cell_resize( eroded_sed , sed_cell_size(eroded_sed)*dx*width );
         
      //sed_hydro_add_cell( river_data  , eroded_sed );
         
//      eh_message( "eroded sediment (m^3): %.3g" , volume_eroded );
         
      sed_cell_destroy( eroded );
   }

   return eroded_sed;
}
*/

gboolean
bed_load_1d_domain( double x , double y , Bed_load_data *user_data )
{
   double y_0   = user_data->y_0;
   double dy    = user_data->dy;
   double r_max = user_data->r_max;

   y*=dy;

   if ( y >= y_0 && y <= y_0+r_max && x>0 )
      return TRUE;
   
   return FALSE;
}

gboolean
bed_load_2d_domain( double x , double y , Bed_load_data *user_data )
{
   double x_0   = user_data->x_0;
   double y_0   = user_data->y_0;
   double dx    = user_data->dx;
   double dy    = user_data->dy;
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

