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

#include "utils.h"
#include "sed_cube.h"

CLASS ( Sed_cube )
{
   gchar* name; //< The name of the Sed_cube.
   double age; //< The age of the Sed_cube.
   double time_step; //< The time step.
   double storm_value; //< The magnitude of the current storm.
   double quake_value; //< The magnitude of the current earthquake.
   double tidal_range; //< The tidal range in meters.
   double tidal_period; //< The tidal period.
   double wave[3]; //< The wave height, period, and length respectively.
   GSList *storm_list;
   GList *in_suspension; //< Array of grids (one for each river) of cells with sediment that is suspended in the water.
   Sed_cell erode; //< Sediment that has been eroded.
   Sed_cell remove; //< Sediment to be removed from the river
   Sed_column **col; //< Array of poiters to columns making up the profile.
   int n_x; //< The number of Sed_columns in the x-direction.
   int n_y; //< The number of Sed_columns in the y-direction.
   double basinWidth; //< The width of the basin.
   double dx; //< The spacing of the columns in the x-direction
   double dy; //< The spacing of the columns in the y-direction
   double sea_level; //< The current height of sea level.
   GList *river; //< Information for each of the river mouths.
   GList *shore; //< A doubly liked list that defines the shore line.
   double cell_height; //< Height of a cell of sediment.
   Sed_constants constants; //< The physical constants for the profile (g, rho_w, etc)
};

GQuark
sed_cube_error_quark( void )
{
   return g_quark_from_static_string( "sed-cube-error-quark" );
}

gboolean is_sedflux_3d( void )
{
   if ( strcasecmp( g_get_application_name( ) , "sedflux3d" )==0 )
      return TRUE;
   else
      return FALSE;
}

#define DEFAULT_BINS (16)

Sed_cube sed_cube_new( gssize n_x , gssize n_y )
{
   Sed_cube s;
   
   s = sed_cube_new_empty( n_x , n_y );

   if ( s )
   {
      gssize i, j;

      for (i=0;i<n_x;i++)
         for (j=0;j<n_y;j++)
         {
            s->col[i][j] = sed_column_new( DEFAULT_BINS );
            sed_column_set_x_position( s->col[i][j] , i );
            sed_column_set_y_position( s->col[i][j] , j );
         }
   }
   
   return s;
}

Sed_cube sed_cube_new_empty( gssize n_x , gssize n_y )
{
   Sed_cube s;
   
   eh_return_val_if_fail( n_x>=0 , NULL );
   eh_return_val_if_fail( n_y>=0 , NULL );

   NEW_OBJECT( Sed_cube , s );

   if ( n_x*n_y>0 )
   {
      gssize i;
      s->col    = eh_new( Sed_column* , n_x     );
      s->col[0] = eh_new( Sed_column  , n_x*n_y );
      for ( i=1 ; i<n_x ; i++ )
         s->col[i] = s->col[i-1]+n_y;
   }
   else
      s->col = NULL;

   s->in_suspension = NULL;

   s->erode  = sed_cell_new_env( );
   s->remove = sed_cell_new_env( );
   
   s->name = g_strdup( "( null )" );

   s->age          = 0.;
   s->time_step    = 0.;
   s->sea_level    = 0.;
   s->storm_value  = 1.;
   s->quake_value  = 0.;
   s->tidal_range  = 0.;
   s->tidal_period = G_MAXFLOAT;
   s->wave[0]      = 1.;
   s->wave[1]      = 1.;
   s->wave[2]      = 1.;
   s->storm_list   = NULL;
   s->dx           = 1.;
   s->dy           = 1.;
   s->n_x          = n_x;
   s->n_y          = n_y;
   s->river        = NULL;
   s->shore        = NULL;
   
   return s;
}

#define SED_KEY_MARGIN_NAME    "margin name"
#define SED_KEY_V_RES          "vertical resolution"
#define SED_KEY_X_RES          "x resolution"
#define SED_KEY_Y_RES          "y resolution"
#define SED_KEY_H_RES          "horizontal resolution"
#define SED_KEY_LENGTH         "basin length"
#define SED_KEY_WIDTH          "basin width"
#define SED_KEY_BATHY_FILE     "bathymetry file"
#define SED_KEY_SEDIMENT_FILE  "sediment file"

Sed_cube
sed_cube_new_from_file( gchar* file )
{
   Sed_cube p = NULL;

   if ( file )
   {
      gchar* name;
      gchar* bathy_file;
      gchar* sediment_file;
      double x_res, y_res, z_res;
      Eh_key_file key_file = eh_key_file_scan( file );

      /* Scan Sed_cube parameters from key-file */
      name          = eh_key_file_get_value    ( key_file , "global" , SED_KEY_MARGIN_NAME );
      z_res         = eh_key_file_get_dbl_value( key_file , "global" , SED_KEY_V_RES );
      x_res         = eh_key_file_get_dbl_value( key_file , "global" , SED_KEY_X_RES );
      y_res         = eh_key_file_get_dbl_value( key_file , "global" , SED_KEY_Y_RES );
      bathy_file    = eh_key_file_get_value    ( key_file , "global" , SED_KEY_BATHY_FILE );
      sediment_file = eh_key_file_get_value    ( key_file , "global" , SED_KEY_SEDIMENT_FILE );

      /* Scan in the sediment and set the environment. */
      {
         Sed_sediment sediment_type = sed_sediment_scan( sediment_file );
         sed_sediment_set_env( sediment_type );
         sed_sediment_destroy( sediment_type );
      }

      /* Create the cube and set positions and elevations. */
      {
         gssize i, j;
         Sed_column this_col;
         Eh_dbl_grid grid;
         GError* err = NULL;

         /* Read the bathymetry.  The method depends if the profile is 1 or 2 D. */
         if ( is_sedflux_3d() )
            grid = sed_get_floor_2d_grid( bathy_file , x_res , y_res );
         else
            grid = sed_get_floor_1d_grid( bathy_file , x_res , y_res , &err );

         if ( err )
         {
            fprintf( stderr , "Unable to read bathymetry: %s" , err->message );
            eh_exit(-1);
         }

         /* Create the cube. */
         p = sed_cube_new( eh_grid_n_x(grid) , eh_grid_n_y(grid) );

         /* Set column positions and elevations. */
         for ( i=0 ; i<sed_cube_n_x(p) ; i++ )
            for ( j=0 ; j<sed_cube_n_y(p) ; j++ )
            {
               this_col = sed_cube_col_ij(p,i,j);

               sed_column_set_x_position ( this_col , eh_grid_x(grid)[i]        );
               sed_column_set_y_position ( this_col , eh_grid_y(grid)[j]        );

               sed_column_set_base_height( this_col , eh_dbl_grid_val(grid,i,j) );
            }

         eh_grid_destroy     ( grid , TRUE   );
      }

      /* Set cube resolutions. */
      sed_cube_set_x_res( p , x_res );
      sed_cube_set_y_res( p , y_res );
      sed_cube_set_z_res( p , z_res );

      /* Set cube name */
      sed_cube_set_name( p , name );

      /* Free resources */
      eh_free             ( name          );
      eh_free             ( bathy_file    );
      eh_free             ( sediment_file );
      eh_key_file_destroy ( key_file      );
   }

   return p;
}

Sed_cube sed_cube_free( Sed_cube s , gboolean free_data )
{
   if ( s )
   {
      GList *list;

      if ( free_data )
      {
         gssize i, n_nodes = sed_cube_size(s);
         for ( i=0 ; i<n_nodes ; i++ )
            sed_column_destroy( s->col[0][i] );
      }

      eh_free( s->col[0] );
      eh_free( s->col    );

      for ( list=s->in_suspension ; list ; list=list->next )
         sed_cube_destroy_in_suspension( list->data );

      for ( list=s->river ; list ; list=list->next )
         sed_destroy_river( list->data );

      sed_cell_destroy( s->erode  );
      sed_cell_destroy( s->remove );

      sed_cube_destroy_storm_list( s );

      g_list_free( s->shore );

      eh_free( s->name );
      eh_free( s );
   }

   return NULL;
}

Sed_cube sed_cube_free_river( Sed_cube p )
{
   if ( p )
   {
      GList *list;
      for ( list=p->river ; list ; list=list->next )
         sed_destroy_river( list->data );
      p->river = NULL;
   }
   return p;
}

Sed_cube sed_cube_destroy( Sed_cube s )
{
   sed_cube_free( s , TRUE );
   return NULL;
}

Sed_cube sed_cube_copy_scalar_data( Sed_cube dest , const Sed_cube src )
{
   if ( !dest )
      dest = sed_cube_new( src->n_x , dest->n_y );

   eh_free( dest->name );
   dest->name = g_strdup( src->name );

   dest->age          = src->age;
   dest->time_step    = src->time_step;
   dest->storm_value  = src->storm_value;
   dest->quake_value  = src->quake_value;
   dest->tidal_range  = src->tidal_range;
   dest->tidal_period = src->tidal_period;
   dest->wave[0]      = src->wave[0];
   dest->wave[1]      = src->wave[1];
   dest->wave[2]      = src->wave[2];
   dest->sea_level    = src->sea_level;
   dest->cell_height  = src->cell_height;
   dest->dx           = src->dx;
   dest->dy           = src->dy;

   return dest;
}

Sed_cube sed_cube_copy_pointer_data( Sed_cube dest , const Sed_cube src )
{
   if ( !dest )
      dest = sed_cube_new( src->n_x , dest->n_y );

   dest->in_suspension = src->in_suspension;
   dest->erode         = src->erode;
   dest->remove        = src->remove;
   dest->storm_list    = g_slist_copy( src->storm_list );

   return dest;
}

Sed_cell_grid sed_cube_create_in_suspension( Sed_cube s )
{
   Sed_cell_grid in_suspension;

   if ( s )
   {
      in_suspension = sed_cell_grid_new( 2*s->n_x , 2*s->n_y );

      sed_cell_grid_init( in_suspension , sed_sediment_env_size() );

      //---
      // in_suspension is twice as large in each dimension than the cube.  the
      // indices are adjusted so that (0,0) is the middle of the grid.  thus,
      // the grid runs from -n_x -> n_x-1.
      //---
      eh_grid_reindex( in_suspension , -s->n_x , -s->n_y );
   }

   return in_suspension;
}

Sed_cell_grid sed_cube_destroy_in_suspension( Sed_cell_grid g )
{
   sed_cell_grid_free( g );
   eh_grid_destroy( g , TRUE );
   return NULL;
}

GSList* sed_cube_storm_list( Sed_cube c )
{
   eh_return_val_if_fail( c!=NULL , NULL );
   return c->storm_list;
}

Sed_cube sed_cube_set_storm_list( Sed_cube c , GSList *storms )
{
   sed_cube_destroy_storm_list( c );
   c->storm_list = storms;
   return c;
}

void __free_slist_ocean_storm_data( Sed_ocean_storm s , gpointer user_data )
{
   sed_ocean_storm_destroy( s );
}

Sed_cube sed_cube_destroy_storm_list( Sed_cube c )
{
   eh_return_val_if_fail( c!=NULL , NULL );

   g_slist_foreach( c->storm_list                         ,
                    (GFunc)&__free_slist_ocean_storm_data ,
                    NULL );

   g_slist_free( c->storm_list );

   return c;
}

Sed_cube sed_cube_set_bathy( Sed_cube c , Eh_dbl_grid g )
{
   eh_return_val_if_fail( g!=NULL , c );

   if ( !c )
      c = sed_cube_new( eh_grid_n_x(g) , eh_grid_n_y(g) );

   eh_require( eh_grid_n_x(g)==c->n_x );
   eh_require( eh_grid_n_y(g)==c->n_y );

   if ( g )
   {
      gssize i, j;
      for ( i=0 ; i<c->n_x ; i++ )
         for ( j=0 ; j<c->n_y ; j++ )
            sed_column_set_base_height( c->col[i][j] , eh_dbl_grid_val(g,i,j) );
   }

   return c;
}

Sed_cube sed_cube_set_dz( Sed_cube p , double new_dz )
{
   eh_return_val_if_fail( p!=NULL , NULL );

   {
      gssize i, len = sed_cube_size(p);
      p->cell_height = new_dz;
      for ( i=0 ; i<len ; i++ )
         sed_column_set_z_res( sed_cube_col( p , i ) , new_dz );
   }

   return p;
}

char* sed_cube_name( const Sed_cube s )
{
   eh_return_val_if_fail( s!=NULL , NULL );
   return g_strdup( s->name );
}

gssize sed_cube_size( const Sed_cube s )
{
   eh_return_val_if_fail( s!=NULL , 0 );
   return s->n_x*s->n_y;
}

gssize sed_cube_n_x( const Sed_cube s )
{
   eh_return_val_if_fail( s!=NULL , 0 );
   return s->n_x;
}

gssize sed_cube_n_y( const Sed_cube s )
{
   eh_return_val_if_fail( s!=NULL , 0 );
   return s->n_y;
}

Sed_column sed_cube_col( const Sed_cube s , gssize ind )
{
   eh_return_val_if_fail( s!=NULL , NULL );

   eh_require( ind>=0 );
   eh_require( ind<sed_cube_size(s) );

   return s->col[0][ind];
}

Sed_column sed_cube_col_ij( const Sed_cube s , gssize i , gssize j )
{
   eh_return_val_if_fail( s!=NULL , NULL );

   eh_require( i>=0 );
   eh_require( j>=0 );
   eh_require( i<sed_cube_n_x(s) );
   eh_require( j<sed_cube_n_y(s) );

   return s->col[i][j];
}

double sed_cube_sea_level( const Sed_cube s )
{
   eh_return_val_if_fail( s!=NULL , 0. );
   return s->sea_level;
}

double* sed_cube_x( const Sed_cube s , gssize *id )
{
   double *x;

   eh_return_val_if_fail( s!=NULL , NULL );

   {
      gssize i, n_x;

      if ( !id )
      {
         x = eh_new( double , s->n_x );
         for ( i=0 ; i<s->n_x ; i++ )
            x[i] = sed_column_x_position( sed_cube_col(s,i*s->n_y) );
      }
      else
      {
         for ( n_x=0 ; id[n_x]>=0 ; n_x++ );
         x = eh_new( double , n_x );
         for ( i=0 ; i<n_x ; i++ )
            x[i] = sed_column_x_position( sed_cube_col(s,id[i]) );
      }
   }

   return x;
}

double* sed_cube_y( const Sed_cube s , gssize *id )
{
   double *y;

   eh_return_val_if_fail( s!=NULL , NULL );

   {
      gssize i, n_y;

      if ( !id )
      {
         y = eh_new( double , s->n_y );
         for ( i=0 ; i<s->n_y ; i++ )
            y[i] = sed_column_y_position( sed_cube_col(s,i) );
      }
      else
      {
         for ( n_y=0 ; id[n_y]>=0 ; n_y++ );
         y = eh_new( double , n_y );
         for ( i=0 ; i<n_y ; i++ )
            y[i] = sed_column_y_position( sed_cube_col(s,id[i]) );
      }
   }

   return y;
}

double sed_cube_col_x( const Sed_cube s , gssize id )
{
   eh_require( id >= 0 );
   eh_require( id < sed_cube_size(s) );
if ( id<0 )
   eh_watch_int( id );
   return sed_column_x_position( sed_cube_col(s,id) );
}

double sed_cube_col_x_ij( const Sed_cube s , gssize i , gssize j )
{
   return sed_column_x_position( sed_cube_col_ij(s,i,j) );
}

double sed_cube_col_y( const Sed_cube s , gssize id )
{
   eh_require( id >= 0 );
   eh_require( id < sed_cube_size(s) );
if ( id<0 )
   eh_watch_int( id );
   return sed_column_y_position( sed_cube_col(s,id) );
}

double sed_cube_col_y_ij( const Sed_cube s , gssize i , gssize j )
{
   return sed_column_y_position( sed_cube_col_ij(s,i,j) );
}

double sed_cube_x_slope( const Sed_cube s , gssize i , gssize j )
{
   double slope;

   eh_require( s );
   eh_require( is_in_domain( s->n_x , s->n_y , i , j ) );

   if ( s->n_x < 2 )
      return 0;

   if ( i==s->n_x-1 )
      slope = ( sed_cube_water_depth( s , i   , j )
              - sed_cube_water_depth( s , i-1 , j ) )
            / sed_cube_x_res( s );
   else
      slope = ( sed_cube_water_depth( s , i+1 , j )
              - sed_cube_water_depth( s , i   , j ) )
            / sed_cube_x_res( s );

   return slope;
}

double sed_cube_y_slope( const Sed_cube s , gssize i , gssize j )
{
   double slope;

   eh_require( s );
   eh_require( is_in_domain( s->n_x , s->n_y , i , j ) );

   if ( s->n_y < 2 )
      return 0;

   if ( j==s->n_y-1 )
      slope = ( sed_cube_water_depth( s , i , j   )
              - sed_cube_water_depth( s , i , j-1 ) )
            / sed_cube_y_res( s );
   else
      slope = ( sed_cube_water_depth( s , i , j+1 )
              - sed_cube_water_depth( s , i , j   ) )
            / sed_cube_y_res( s );

   return slope;
}

double sed_cube_y_slope_fast( const Sed_cube s , gssize i , gssize j )
{
   eh_require( s );
   eh_require( is_in_domain( s->n_x , s->n_y , i , j ) );

   return (   sed_cube_water_depth( s , i , j+1 )
            - sed_cube_water_depth( s , i , j   ) )
          / sed_cube_y_res( s );
}

double sed_cube_slope( const Sed_cube s , gssize i , gssize j )
{
   double dx, dy;

   dx = sed_cube_x_slope( s , i , j );
   dy = sed_cube_y_slope( s , i , j );

   return sqrt( dx*dx + dy*dy );
}

double sed_cube_slope_dir( const Sed_cube s , gssize i , gssize j )
{
   double dx, dy;

   dx = sed_cube_x_slope( s , i , j );
   dy = sed_cube_y_slope( s , i , j );

   return atan2( dy , dx );
}

Eh_pt_2 sed_cube_slope_vector( const Sed_cube s , gssize i , gssize j )
{
   Eh_pt_2 u;

   eh_require( s );

   u.x = sed_cube_x_slope( s , i , j );
   u.y = sed_cube_y_slope( s , i , j );

   return u;
}

double sed_cube_top_height(const Sed_cube p , gssize i , gssize j )
{
   return sed_column_top_height( sed_cube_col_ij( p , i , j ) );
}

Eh_ind_2 sed_ind2sub( gssize ind , gssize n_y )
{
   Eh_ind_2 sub;

   sub.i = floor( ind / n_y );
   sub.j = fmod ( ind , n_y );

   return sub;
}

Eh_dbl_grid sed_cube_grid( const Sed_cube s   ,
                           Sed_grid_func func ,
                           gssize *index )
{
   Eh_dbl_grid g;

   eh_require( s );

   {
      gssize i, j;

      g = eh_grid_new( double , s->n_x , s->n_y );

      if ( index )
      {
         for ( i=0 ; index[i]>=0 ; i++ )
            eh_dbl_grid_set_val( g , 0 , index[i] , (*func)( s , 0 , index[i] ) );
      }
      else
      {
         for ( i=0 ; i<eh_grid_n_x(g) ; i++ )
            for ( j=0 ; j<eh_grid_n_y(g) ; j++ )
               eh_dbl_grid_set_val( g , i , j , (*func)( s , i , j ) );
      }
   }

   return g;
}

Eh_dbl_grid sed_cube_slope_dir_grid( const Sed_cube s , gssize *index )
{
   return sed_cube_grid( s , S_SLOPE_DIR_FUNC , index );
}

Eh_dbl_grid sed_cube_x_slope_grid( const Sed_cube s , gssize *index )
{
   return sed_cube_grid( s , S_X_SLOPE_FUNC , index );
}

Eh_dbl_grid sed_cube_y_slope_grid( const Sed_cube s , gssize *index )
{
   return sed_cube_grid( s , S_Y_SLOPE_FUNC , index );
}

Eh_dbl_grid sed_cube_water_depth_grid( const Sed_cube s , gssize *index )
{
   return sed_cube_grid( s , S_WATER_DEPTH_FUNC , index );
}

Eh_dbl_grid sed_cube_thickness_grid( const Sed_cube s , gssize *index )
{
   return sed_cube_grid( s , S_THICKNESS_FUNC , index );
}

Eh_dbl_grid sed_cube_load_grid( const Sed_cube s , gssize *index )
{
   return sed_cube_grid( s , S_LOAD_FUNC , index );
}

Sed_river *sed_cube_river_by_name( Sed_cube s , const char *name )
{
   Sed_river* the_river = NULL;

   if ( s )
   {
      GList *found;
      found = g_list_find_custom( s->river ,
                                  name     ,
                                  (GCompareFunc)&sed_cmp_river_name );
      if ( found )
         the_river = (Sed_river*)(found->data);
   }

   return the_river;
}

gint sed_cmp_river_name( Sed_river *this_river , const char *name )
{
   if ( this_river->river_name )
      return g_ascii_strcasecmp( this_river->river_name , name );
   else
      return -1;
}

Sed_river *sed_cube_river( Sed_cube s , gssize n )
{
   eh_return_val_if_fail( s!=NULL , NULL );
   return g_list_nth_data( s->river , n );
}

gssize sed_cube_river_id( Sed_cube s , Sed_river* river )
{
   return g_list_index( s->river , river );
}

Sed_cell_grid sed_cube_in_suspension( Sed_cube s , gssize river_no )
{
   eh_return_val_if_fail( s!=NULL , NULL );
   return g_list_nth_data( s->in_suspension , river_no );
}

GList* sed_cube_river_list( Sed_cube s )
{
   return s->river;
}

gssize sed_cube_number_of_rivers( Sed_cube s )
{
   eh_return_val_if_fail( s!=NULL , 0 );
   return g_list_length( s->river );
}

double sed_cube_time_step( const Sed_cube s )
{
   eh_require( s );
   return s->time_step;
}

double sed_cube_time_step_in_years( const Sed_cube s )
{
   eh_require( s );
   return s->time_step;
}

double sed_cube_time_step_in_seconds( const Sed_cube s )
{
   eh_require( s );
   return s->time_step*S_SECONDS_PER_YEAR;
}

double sed_cube_time_step_in_days( const Sed_cube s )
{
   eh_require( s );
   return s->time_step*S_DAYS_PER_YEAR;
}

Sed_constants sed_cube_constants( const Sed_cube s )
{
   eh_require( s );
   return s->constants;
}

double sed_cube_x_res( const Sed_cube s )
{
   eh_require( s );
   return s->dx;
}

double sed_cube_y_res( const Sed_cube s )
{
   eh_require( s );
   return s->dy;
}

double sed_cube_z_res( const Sed_cube s )
{
   eh_require( s );
   return s->cell_height;
}

Sed_cell sed_cube_to_remove( const Sed_cube s )
{
   eh_require( s );
   return s->remove;
}

Sed_cell sed_cube_to_add( const Sed_cube s )
{
   eh_require( s );
   return s->erode;
}

gssize sed_column_x_index( const Sed_cube c , double x )
{
   gssize i;

   eh_require( c );

   if ( x<=sed_cube_col_x( c,0 ) )
      i = 0;
   else if ( x>=sed_cube_col_x( c , (c->n_y-1)*c->n_x ) )
      i = c->n_x-1;
   else
   {
      for ( i=0 ; i<c->n_x && sed_column_x_position( c->col[i][0] ) <= x ; i++ );
      i = i-1;
   }

   return i;
}

gssize sed_column_y_index( const Sed_cube c , double y )
{
   gssize j;

   eh_require( c );

   if ( y<=sed_cube_col_y( c,0 ) )
      j = 0;
   else if ( y>=sed_cube_col_y( c , c->n_y-1 ) )
      j = c->n_y-1;
   else
   {
      for ( j=0 ; j<c->n_y && sed_column_y_position( c->col[0][j] ) <= y ; j++ );
      j = j-1;
   }

   return j;
}

double sed_cube_wave_height( const Sed_cube s )
{
   return s->wave[0];
}

double sed_cube_wave_period( const Sed_cube s )
{
   return s->wave[1];
}

double sed_cube_wave_length( const Sed_cube s )
{
   return s->wave[2];
}

double sed_cube_storm( const Sed_cube s )
{
   return s->storm_value;
}

double sed_cube_quake( const Sed_cube s )
{
   return s->quake_value;
}

double sed_cube_age( const Sed_cube s )
{
   return s->age;
}

double sed_cube_age_in_years( const Sed_cube s )
{
   return s->age;
}

double sed_cube_tidal_period( const Sed_cube s )
{
   return s->tidal_period;
}

Sed_cube sed_cube_set_tidal_period( Sed_cube s , double new_val )
{
   s->tidal_period = new_val;
   return s;
}

double sed_cube_tidal_range( const Sed_cube s )
{
   return s->tidal_range;
}

Sed_cube sed_cube_set_tidal_range( Sed_cube s , double new_val )
{
   s->tidal_range = new_val;
   return s;
}

double sed_cube_water_depth( const Sed_cube p , gssize i , gssize j )
{
   double depth;

   eh_require( p );

   depth = sed_column_water_depth( sed_cube_col_ij(p,i,j) );

   return depth;
}

double sed_cube_water_pressure( const Sed_cube p , gssize i , gssize j )
{
   double press = 0.;

   eh_require( p );

   {
      double water_depth =  sed_cube_water_depth(p,i,j);

      if ( water_depth > 0 )
         press = water_depth*sed_rho_sea_water()*sed_gravity();
   }

   return press;
}

double sed_cube_load( const Sed_cube p , gssize i , gssize j )
{
   double load;

   eh_require( p );

   {
      double sediment_load = sed_column_load_at( sed_cube_col_ij(p,i,j) , 0 );
      double  water_load   = sed_cube_water_pressure( p , i , j );

      load = water_load + sediment_load;
   }

   return load;
}

double sed_cube_thickness( const Sed_cube p , gssize i , gssize j )
{
   return sed_column_thickness( p->col[i][j] );
}

double sed_cube_base_height( const Sed_cube p , gssize i , gssize j )
{
   return sed_column_base_height( sed_cube_col_ij(p,i,j) );
}

/** Duplicate a sediment cube.

   \param c The cube to duplicate.

   \return A newly created cube.

   \callgraph
*/
Sed_cube sed_cube_dup( Sed_cube c )
{
   return sed_cube_copy( NULL , c );
}

/** Copy a sediment cube.

   \param src  The cube to duplicate.
   \param dest Location of the duplicate cube (or NULL to create a cube);

   \return Location of the duplicate cube.

   \callgraph
*/
Sed_cube sed_cube_copy( Sed_cube dest , const Sed_cube src )
{
   eh_require( src );

   if ( dest==NULL )
      dest = sed_cube_new( src->n_x , src->n_y );

   eh_require( dest->n_x==src->n_x && dest->n_y==src->n_y );

   sed_cube_copy_scalar_data ( dest , src );
   sed_cube_copy_pointer_data( dest , src );

   return dest;
}

/** Place columns from a Sed_cube into a new 1D Sed_cube

Create a new 1D Sed_cube out of Sed_columns of an existing Sed_cube.  Columns
are specified by their x and y indices as well as an elevation.  Everything above
the elevation is copied to a new Sed_column and placed in the new Sed_cube.

@param src A Sed_cube
@param x   x-indices of the columns to copy
@param y   y-indices of the columns to copy
@param z   Vertical position of the new column
@param len Length of x, y, and z

@return A new Sed_cube containing a copy of the specified columns.
*/
Sed_cube sed_cube_copy_cols( const Sed_cube src , gssize* x , gssize* y , double* z , gssize len )
{
   Sed_cube new_cube = NULL;

   eh_require( src   );
   eh_require( x     );
   eh_require( y     );
   eh_require( z     );
   eh_require( len>0 );

   new_cube = sed_cube_new( 1 , len );

   {
      gssize i;

      sed_cube_copy_scalar_data( new_cube , src );

      for (i=0;i<len;i++)
         new_cube->col[0][i] = sed_column_height_copy( src->col[x[i]][y[i]] ,
                                                       z[i]                 ,
                                                       NULL );
   }

   return new_cube;
}

/** Place columns from a Sed_cube into a new 1D Sed_cube

Create a new 1D Sed_cube out of Sed_columns of an existing Sed_cube.  Columns
are specified by their x and y positions as well as an elevation.  Everything above
the elevation is copied to a new Sed_column and placed in the new Sed_cube.

@param src A Sed_cube
@param x   x-positions of the columns to copy
@param y   y-positions of the columns to copy
@param z   Vertical position of the new column
@param len Length of x, y, and z

@return A new Sed_cube containing a copy of the specified columns.
*/
Sed_cube sed_cube_copy_line( const Sed_cube src , double *x , double *y , double *z , gssize len )
{
   Sed_cube new_cube = NULL;

   eh_require( src   );
   eh_require( x     );
   eh_require( y     );
   eh_require( z     );
   eh_require( len>0 );

   {
      gssize i;
      gssize *id = eh_new( gssize , len );

      for ( i=0 ; i<len ; i++ )
         id[i] = sed_cube_column_id( src , x[i] , y[i] );

      new_cube = sed_cube_new( 1 , len );

      sed_cube_copy_scalar_data( new_cube , src );

      for (i=0;i<len;i++)
         new_cube->col[0][i] = sed_column_height_copy( src->col[0][id[i]] ,
                                                       z[i] ,
                                                       NULL );
      eh_free( id );
   }

   return new_cube;
}

Sed_cube sed_cube_cols( Sed_cube src , gssize *path )
{
   Sed_cube new_cube = NULL;

   eh_require( src!=NULL );

   {
      gssize j;
      gssize len;

      if ( path )
         for ( len=0 ; path[len]>=0 ; len++ );
      else
         len = sed_cube_size( src );

      new_cube = sed_cube_new_empty( 1 , len );
      sed_cube_copy_scalar_data( new_cube , src );

      if ( path )
         for ( j=0 ; j<len ; j++ )
            new_cube->col[0][j] = src->col[0][path[j]];
      else
         for ( j=0 ; j<len ; j++ )
            new_cube->col[0][j] = src->col[0][j];
   }

   return new_cube;
}

gssize sed_cube_river_mouth_1d( Sed_cube c )
{
   gssize i_river = 0;

   eh_require( c );
   eh_require( c->n_x==1 );

   {
      double river_depth = 1e-5;

      //---
      // Search to the right
      //---
      for ( i_river=0 ;
            i_river<c->n_y && sed_cube_water_depth(c,0,i_river) <= river_depth ;
            i_river++ );

      if ( i_river >= c->n_y )
         eh_warning( "The river mouth is at the right boundary" );
      else if ( i_river <= 0 )
         eh_warning( "The river mouth is at the left boundary" );

      eh_clamp( i_river , 0 , c->n_y-1 );
   }

   return i_river;
}

Sed_cube sed_cube_remove( Sed_cube dest , Sed_cube src )
{
   gssize i;
   gssize dest_size = sed_cube_size(dest);
   gssize *src_id = eh_new( gssize , dest_size );

   for ( i=0 ; i<dest_size ; i++ )
      src_id[i] = sed_cube_column_id( src ,
                                      sed_column_x_position( dest->col[0][i] ) ,
                                      sed_column_y_position( dest->col[0][i] ) );

   for ( i=0 ; i<dest_size ; i++ )
      if ( src_id[i]>=0 )
         sed_column_remove( dest->col[0][i] ,
                            src->col[0][src_id[i]] );

   eh_free( src_id );

   return dest;
}

Sed_cube sed_cube_add( Sed_cube dest , const Sed_cube src )
{
   gssize i;
   gssize src_size  = sed_cube_size(src);
   gssize *dest_id = eh_new( gssize , src_size );

   for ( i=0 ; i<src_size ; i++ )
      dest_id[i] = sed_cube_column_id( dest ,
                                       sed_column_x_position( src->col[0][i] ) ,
                                       sed_column_y_position( src->col[0][i] ) );

   for ( i=0 ; i<src_size ; i++ )
      if ( dest_id[i]>=0 )
         sed_column_add( dest->col[0][dest_id[i]] ,
                                   src->col[0][i] );

   eh_free( dest_id );

   return dest;
}

double sed_cube_mass( const Sed_cube p )
{
   double mass = 0;

   if ( p )
   {
      gssize i;
      gssize len = sed_cube_size(p);

      for ( i=0 ; i<len ; i++ )
         mass += sed_column_mass( sed_cube_col(p,i) );
      mass *= sed_cube_x_res(p)*sed_cube_y_res(p);
   }

   return mass;
}

double sed_cube_mass_in_suspension( const Sed_cube p )
{
   double mass = 0.;

   if ( p )
   {
      gssize i;
      gssize n_rivers = sed_cube_number_of_rivers(p);
      Sed_cell_grid in_suspension;

      for ( i=0 ; i<n_rivers ; i++ )
      {
         in_suspension = sed_cube_in_suspension(p,i);
         mass += sed_cell_grid_mass( in_suspension );
      }
      mass *= sed_cube_x_res(p)*sed_cube_y_res(p);
   }

   return mass;
}

Sed_cube sed_cube_set_sea_level( Sed_cube s , double new_sea_level )
{
   if ( s )
   {
      gssize i, len = sed_cube_size(s);
      s->sea_level = new_sea_level;
      for ( i=0 ; i<len ; i++ )
         sed_column_set_sea_level( sed_cube_col(s,i) , new_sea_level );
   }

   return s;
}

Sed_cube sed_cube_adjust_sea_level( Sed_cube s , double dz )
{
   return sed_cube_set_sea_level( s , s->sea_level + dz );
}

Sed_cube sed_cube_set_base_height( Sed_cube s , gssize i , gssize j , double height )
{
   sed_column_set_base_height( sed_cube_col_ij(s,i,j) , height );
   return s;
}

Sed_cube sed_cube_adjust_base_height( Sed_cube s , gssize i , gssize j , double dz )
{
   Sed_column this_col = sed_cube_col_ij(s,i,j);
   sed_column_set_base_height( this_col ,
                               sed_column_base_height( this_col ) + dz );
   return s;
}

Sed_cube sed_cube_set_nth_river( Sed_cube s , gssize n , Sed_river *new_river )
{
   eh_require( s )
   {
      Sed_river *nth = g_list_nth( s->river , n )->data;
      memcpy( nth , new_river , sizeof(Sed_river) );
   }
   return s;
}

Eh_pt_2 *sed_cube_river_mouth_position( Sed_cube s ,
                                        Sed_river *this_river )
{
   Eh_pt_2 *river_mouth_pos;

   eh_require( s );
   eh_require( this_river );

   {
      Eh_polygon_2 shore_cell = eh_get_rectangle_polygon(
                                   eh_create_pt_2(
                                      (this_river->x_ind+.5)*sed_cube_x_res( s ) ,
                                      (this_river->y_ind+.5)*sed_cube_y_res( s ) ) ,
                                   sed_cube_x_res( s )                             ,
                                   sed_cube_y_res( s ) );
      GList *river_end = eh_find_polygon_crossings( 
                            eh_create_pt_2(
                               (this_river->hinge->x+.5)*sed_cube_x_res( s ) ,
                               (this_river->hinge->y+.5)*sed_cube_y_res( s ) ) ,
                            this_river->hinge->angle                           ,
                            shore_cell                                         ,
                            POLYGON_IN_CROSSINGS ); 

      if ( river_end && (g_list_length( river_end ) == 1) )
         river_mouth_pos = (Eh_pt_2*)(river_end->data);
      else
      {
         river_mouth_pos = NULL;
         eh_require_not_reached( );
      }

      eh_destroy_polygon( shore_cell );
      g_list_free( river_end );
   }

   return river_mouth_pos;
}

Sed_hydro sed_cube_river_data( Sed_cube s , GList *this_river )
{
   return ((Sed_river*)(this_river->data))->data;
}

Sed_cube sed_cube_set_river_data( Sed_cube s        ,
                                  GList *this_river ,
                                  Sed_hydro new_river_data )
{
   Sed_hydro this_hydro_rec = sed_cube_river_data( s , this_river );
   sed_hydro_copy( this_hydro_rec , new_river_data );
   return s;
}

Sed_cube sed_cube_set_river_list( Sed_cube s , GList* river_list )
{
   s->river = river_list;
   return s;
}

Sed_cube sed_cube_add_river( Sed_cube s , Sed_river *new_river )
{
   Sed_cell_grid new_grid;

   new_grid = sed_cube_create_in_suspension( s );

   s->river = g_list_prepend( s->river , new_river );
   s->in_suspension = g_list_prepend( s->in_suspension , new_grid );

   return s;
}

Sed_cube sed_cube_remove_river( Sed_cube s , gssize river_no )
{
   Sed_cell_grid this_grid = g_list_nth_data( s->in_suspension , river_no );

   sed_cube_destroy_in_suspension( this_grid );
   s->in_suspension = g_list_delete_link( s->in_suspension ,
                                          g_list_nth( s->in_suspension ,
                                                      river_no ) );
   return s;
}

void sed_cube_set_nth_river_data( Sed_cube s , int n , Sed_hydro new_data )
{
   Sed_river *nth = g_list_nth( s->river , n )->data;
   sed_hydro_copy( nth->data , new_data );
}

Sed_cube sed_cube_set_name( Sed_cube s , char *name )
{
   eh_require( s )
   {
      eh_free( s->name );
      s->name = g_strdup( name );
   }
   return s;
}

Sed_cube sed_cube_set_time_step( Sed_cube s , double time_step_in_years )
{
   eh_require( s )
   {
      s->time_step = time_step_in_years;
   }
   return s;
}

Sed_cube set_cube_set_constants( Sed_cube s , Sed_constants new_c )
{
   eh_require( s )
   {
      s->constants = new_c;
   }
   return s;
}

Sed_cube sed_cube_set_x_res( Sed_cube s , double new_x_res )
{
   eh_require( s )
   {
      s->dx = new_x_res;
   }
   return s;
}

Sed_cube sed_cube_set_y_res( Sed_cube s , double new_y_res )
{
   eh_require( s )
   {
      s->dy = new_y_res;
   }
   return s;
}

Sed_cube sed_cube_set_z_res( Sed_cube s , double new_z_res )
{
   eh_require( s )
   {
      gssize i, len = sed_cube_size(s);
      for ( i=0 ; i<len ; i++ )
         sed_column_set_z_res( sed_cube_col(s,i) , new_z_res );
      s->cell_height = new_z_res;
   }

   return s;
}

Sed_cube sed_cube_set_wave_height( Sed_cube s , double new_wave_height )
{
   s->wave[0] = new_wave_height;
   return s;
}

Sed_cube sed_cube_set_wave_period( Sed_cube s , double new_wave_period )
{
   s->wave[1] = new_wave_period;
   return s;
}

Sed_cube sed_cube_set_wave_length( Sed_cube s , double new_wave_length )
{
   s->wave[2] = new_wave_length;
   return s;
}

Sed_cube sed_cube_set_storm( Sed_cube s , double new_storm_value )
{
   s->storm_value = new_storm_value;
   return s;
}

Sed_cube sed_cube_set_quake( Sed_cube s , double new_quake_value )
{
   eh_require( s )
   {
      s->quake_value = new_quake_value;
   }
   return s;
}

Sed_cube sed_cube_set_age( Sed_cube s , double new_age )
{
   eh_require( s )
   {
      s->age = new_age;
   }
   return s;
}

Sed_cube sed_cube_adjust_age( Sed_cube s , double dt )
{
   return sed_cube_set_age( s , s->age + dt );
}

Sed_cube sed_cube_increment_age( Sed_cube s )
{
   return sed_cube_adjust_age( s , sed_cube_time_step(s) );
}

Sed_hydro sed_cube_nth_river_data( Sed_cube s , int n )
{
   Sed_hydro hydro_data = NULL;
   eh_require( s )
   {
      Sed_river *nth = g_list_nth( s->river , n )->data;
      hydro_data = nth->data;
   }
   return hydro_data;
}
/*
Sed_cube sed_cube_add_river( Sed_cube s , Sed_river *river )
{
   eh_require( s )
   {
      s->river = g_list_prepend( s->river , river );
   }
   return s;
}
*/

#define VARY_COLS (0)
#define VARY_ROWS (1)

void sed_cube_set_shore( Sed_cube s )
{
   int i;
   Eh_ind_2 *pos;
   GList *shore_list=NULL;
   GList *list;

   g_list_free( s->shore );
   s->shore = NULL;

   //---
   // Look for the shoreline along the first and last rows, then along the
   // first and last columns.  If there are no shore cells along the boundaries
   // then look in the interior.
   //---
   if (    (pos=sed_cube_find_shore( s , 0        , VARY_COLS ))
        || (pos=sed_cube_find_shore( s , s->n_x-1 , VARY_COLS ))
        || (pos=sed_cube_find_shore( s , 0        , VARY_ROWS ))
        || (pos=sed_cube_find_shore( s , s->n_y-1 , VARY_ROWS )) )
      shore_list = sed_cube_find_shore_line( s , pos );
   else
   {
      for ( i=1 ; pos==NULL && i<s->n_x-1 ; i++ )
         pos = sed_cube_find_shore( s , i , VARY_COLS );
      if ( !pos )
         eh_message( "There are no shore cells in the domain" );
      else
         shore_list = sed_cube_find_shore_line( s , pos );
   }

   for ( list=shore_list ; list ; list=list->next )
   {
      fprintf( stderr , "shore : %d , %d\n" ,
         ((Eh_ind_2*)(list->data))->i ,
         ((Eh_ind_2*)(list->data))->j );
   }

   s->shore = shore_list;
}

GList *sed_cube_find_shore_line( Sed_cube s , Eh_ind_2 *pos )
{
   return sed_find_next_shore( NULL , s , pos , g_list_prepend( NULL , pos ) );
}

int eh_compare_ind_2( Eh_ind_2 *a , Eh_ind_2 *b );

GList *sed_find_next_shore( GList *shore_list ,
                            Sed_cube s        ,
                            Eh_ind_2 *pos     ,
                            GList *ignore_list )
{
   int i;
   Eh_ind_2 shift[8] = { {-1,-1} , {0,-1} , {1,-1} ,
                         {-1, 0} ,          {1, 0} ,
                         {-1, 1} , {0, 1} , {1, 1} };
   GList *child_list = NULL;
   GList *list;
   Eh_ind_2 shift_pos;
   Eh_ind_2 *next_pos;

   for ( i=0 ; i<8 ; i++ )
   {
      shift_pos = eh_ind_2_create( pos->i+shift[i].i , pos->j+shift[i].j );
      if (    g_list_find_custom( ignore_list ,
                                  &shift_pos  ,
                                  (GCompareFunc)&eh_compare_ind_2 )
           == NULL
           && is_shore_cell( s , shift_pos.i , shift_pos.j ) )
      {
         next_pos = eh_ind_2_dup( &shift_pos , NULL );
         child_list = g_list_prepend( child_list , next_pos );
      }
   }


   if ( child_list )
   {
      for ( list=child_list ; list ; list=list->next )
         ignore_list = g_list_prepend( ignore_list ,
                                       eh_ind_2_dup( list->data , NULL ) );

      for ( list=child_list ; list ; list=list->next )
         shore_list = sed_find_next_shore( shore_list  ,
                                           s           ,
                                           list->data  ,
                                           ignore_list );
      shore_list = g_list_prepend( shore_list , pos );
   }
   else
      return g_list_prepend( shore_list , pos );

   return shore_list;
}

int eh_compare_ind_2( Eh_ind_2 *a , Eh_ind_2 *b )
{
   if ( a->i==b->i && a->j==b->j )
      return 0;
   else if ( a->i < b->i )
      return -1;
   else if ( a->i > b->i )
      return 1;
   else if ( a->j < b->j )
      return -1;
   else
      return 1;
}

/** Test if a land cell borders an ocean cell.

\param s A pointer to a Sed_cube.
\param x Index to fast dimension.
\param y Index to slow dimension.

\return TRUE if the column is on the shore line.
*/
gboolean is_shore_cell( Sed_cube s , gssize x , gssize y )
{
   gboolean is_shore = FALSE;

   eh_require( s!=NULL );

   if ( x<0 || x>=s->n_x || y<0 || y>=s->n_y )
   {
      int west  = y-1;
      int east  = y+1;
      int north = x-1;
      int south = x+1;


      eh_clamp( west  , 0 , s->n_y-1 );
      eh_clamp( east  , 0 , s->n_y-1 );
      eh_clamp( north , 0 , s->n_x-1 );
      eh_clamp( south , 0 , s->n_x-1 );

      //---
      // If the column is below sea level, it isn't the shore.
      // Otherwise, if any of its neighbours is below sea level,
      // then it is a shore column.
      //---
      if ( sed_column_is_below( s->col[x][y] , s->sea_level ) )
         is_shore = FALSE;
      else if (    sed_column_is_below( s->col[x][west]  , s->sea_level )
                || sed_column_is_below( s->col[x][east]  , s->sea_level )
                || sed_column_is_below( s->col[north][y] , s->sea_level )
                || sed_column_is_below( s->col[south][y] , s->sea_level ) )
         is_shore = TRUE;
      else
         is_shore = FALSE;
   }

   return is_shore;
}

int sed_cube_find_shore_edge( Sed_cube s , gssize i , gssize j )
{
   int shore_edge = 0;
   int west  = j-1;
   int east  = j+1;
   int north = i-1;
   int south = i+1;

   eh_require( s!=NULL );

   //---
   // If this column is below sea level, then it is not a shore edge.
   //---
   if (    !is_in_domain( s->n_x , s->n_y , i , j )
        || sed_column_is_below( s->col[i][j] , s->sea_level ) )
      return shore_edge;

   //---
   // Check if each of the sides of the cell is a shore edge.  That is,
   // which neighbour cells are below sea level.  Turn on the appropriate
   // edge bit for each shore edge.
   //---
   if (    is_in_domain( s->n_x , s->n_y , i , west )
        && sed_column_is_below( s->col[i][west]  , s->sea_level ) )
      shore_edge |= S_WEST_EDGE;

   if (    is_in_domain( s->n_x , s->n_y , i , east )
        && sed_column_is_below( s->col[i][east]  , s->sea_level ) )
      shore_edge |= S_EAST_EDGE;

   if (    is_in_domain( s->n_x , s->n_y , i , north )
        && sed_column_is_below( s->col[north][j] , s->sea_level ) )
      shore_edge |= S_NORTH_EDGE;

   if (    is_in_domain( s->n_x , s->n_y , i , south )
        && sed_column_is_below( s->col[south][j] , s->sea_level ) )
      shore_edge |= S_SOUTH_EDGE;
   
   return shore_edge;
}

Eh_ind_2 *sed_cube_find_adjacent_shore_edge( Sed_cube s ,
                                             gssize i   ,
                                             gssize j   ,
                                             int edge )
{
   Eh_ind_2 this_index;
   int new_edge;

   //---
   // Facing this edge, turn to the right and see if this new edge is a shore
   // edge.  If it is, this is the continuation of the current shore edge.
   // The shore turns to the right.
   //---
   new_edge = sed_rotate_direction( edge , -1 );

   this_index.i = i;
   this_index.j = j;

   if ( sed_cube_is_shore_edge( s , this_index.i , this_index.j , new_edge ) )
      return eh_ind_2_dup( &this_index , NULL );

   if ( !is_in_domain( s->n_x , s->n_y , this_index.i , this_index.j ) )
      return NULL;

   //---
   // Move to the new cell and rotate to the left.  Check if this new edge is
   // a shore edge.  If it is, this is the continuation of the shore edge.
   // The shore continues in a straight line.
   //---
   this_index = sed_shift_index_over_edge( this_index.i ,
                                           this_index.j ,
                                           new_edge );
   new_edge   = sed_rotate_direction( edge , +1 );

   if ( sed_cube_is_shore_edge( s , this_index.i , this_index.j , new_edge ) )
      return eh_ind_2_dup( &this_index , NULL );

   //---
   // Move to the next cell.  By elimination, this must the new shore cell.
   // Turn to the left to face the new shore edge.  We could double check that
   // this is a shore edge but if it weren't, the original shore edge would 
   // not have been a shore edge.  Here, the shore has taken a left turn.
   //---
   this_index = sed_shift_index_over_edge( this_index.i , 
                                           this_index.j ,
                                           new_edge );
      
   return eh_ind_2_dup( &this_index , NULL );
}

/** Rotate directions in jumps of 90 degrees.

Given a cardinal direction, find the direction that is rotated by a given
number of 90-degree jumps.  Positive jumps are counter-clockwise.

\param dir A cardinal direction.
\param angle The number of 90-degree jumps to make.

\return A new direction.
*/
int sed_rotate_direction( int dir , int angle )
{
   return dir << (angle%4);
}

/** Check if the specified edge is a shore edge.

Check if the is a shift from above (below) sea level to below (above) sea level
by moving over the given cell's edge.

\param s    A pointer to a Sed_cube.
\param i    First index to a cell of a Sed_cube.
\param j    Second index to a cell of a Sed_cube.
\param edge An edge of a cell.

\return TRUE if the edge is a shore edge, FALSE otherwise.

*/

gboolean sed_cube_is_shore_edge( Sed_cube s , gssize i , gssize j , int edge )
{
   Eh_ind_2 shift = sed_shift_index_over_edge( i , j , edge );
   
   if ( !is_in_domain( s->n_x , s->n_y , shift.i , shift.j ) )
      return FALSE;

   if (   sed_column_is_below( s->col[i][j]             , s->sea_level )
        ^ sed_column_is_below( s->col[shift.i][shift.j] , s->sea_level ) )
      return TRUE;
   else
      return FALSE;
}

/** Get the indices of the cell accross some cell's edge.

\param i       The first index to a cell within a rectangular grid.
\param j       The second index to a cell within a rectangular grid.
\param edge    The edge of the cell over which to move.

\return        The indices of the new cell.
*/
Eh_ind_2 sed_shift_index_over_edge( gssize i , gssize j , int edge )
{
   Eh_ind_2 shift[4] = { {-1, 0} , { 0,+1} , {+1, 0} , { 0,-1} };
   Eh_ind_2 new_index, this_shift;

   if ( edge|S_NORTH_EDGE )
      this_shift = shift[0];
   else if ( edge|S_EAST_EDGE )
      this_shift = shift[1];
   else if ( edge|S_SOUTH_EDGE )
      this_shift = shift[2];
   else if ( edge|S_WEST_EDGE )
      this_shift = shift[3];

   new_index.i = i+this_shift.i;
   new_index.j = j+this_shift.j;

   return new_index;
}

GList *sed_cube_find_columns_custom( Sed_cube s              ,
                                     gssize i                ,
                                     gssize j                ,
                                     gpointer data           ,
                                     Sed_cube_func stop_func ,
                                     Sed_cube_func angle_func )
{
   double angle;
   GList *column_list=NULL;
   Eh_ind_2 shift;
   Eh_pt_2 pos_in_cell;

   //---
   // Begin in the center of the first cell.
   //---
   pos_in_cell.x = .5*s->dx;
   pos_in_cell.y = .5*s->dy;

   eh_require( s );
   eh_require( is_in_domain( s->n_x , s->n_y , i , j ) );

   //---
   // If the first column satisies the stop criterion, return a NULL list.
   // Otherwise, add this column to the first of the list.
   //---
   if ( (*stop_func)( s , i , j , data ) )
      return column_list;

   column_list = g_list_prepend( column_list , s->col[i][j] );

   //---
   // Trace the path while within the domain while the stop criterion is not
   // satisfied.  Also, watch for flow from one cell, to another, and then
   // back the the first cell.  Stop the search once this occurs.
   //---
   while (     is_in_domain( s->n_x , s->n_y , i , j )
           && !stop_func( s , i , j , data ) )
   {

      // Get the new path direction.
      angle = angle_func( s , i , j , NULL );

      // Using this direction, find the exit point of the cell.
      pos_in_cell = get_path_exit_pos( pos_in_cell , angle , s->dx , s->dy );

      eh_clamp( pos_in_cell.y , .01*s->dy , .99*s->dy );
      eh_clamp( pos_in_cell.x , .01*s->dx , .99*s->dx );

      // From the exit position, get the index shift to move to the next cell.
      // Also, find the position within the new cell.
      shift = get_shift_from_exit_pos( pos_in_cell , s->dx , s->dy );
      pos_in_cell = get_path_entrance_pos( pos_in_cell , s->dx , s->dy );

      eh_clamp( pos_in_cell.y , .01*s->dy , .99*s->dy );
      eh_clamp( pos_in_cell.x , .01*s->dx , .99*s->dx );

      // The indices of the new cell.
      i += shift.i;
      j += shift.j;

      // If this new cell is within the domain, add the column to the list of
      // columns.  To avoid a infinite loop, stop the search if this cell is
      // already in the list.
      if ( is_in_domain( s->n_x , s->n_y , i , j ) )
      {
         if ( column_list->data != s->col[i][j] )
            column_list = g_list_prepend( column_list , s->col[i][j] );
         else
            break;
      }

   }

   return column_list;
}

GList *sed_cube_find_cross_shore_columns( Sed_cube s , gssize i , gssize j )
{
   Eh_pt_2 u;
   GList *column_list = NULL;
   int shift_i, shift_j;
   double max_depth = 20;

   if ( sed_cube_water_depth( s , i , j ) > max_depth )
      return g_list_prepend( column_list , s->col[i][j] );

   u = sed_cube_slope_vector( s , i , j );

   shift_i = eh_sign( u.x );
   shift_j = eh_sign( u.y );

   if ( shift_i != 0 )
      column_list = sed_cube_find_cross_shore_columns( s , i+shift_i , j );

   if ( shift_j != 0 )
      column_list = sed_cube_find_cross_shore_columns( s , i , j+shift_j );

   column_list = g_list_prepend( column_list , s->col[i][j] );

   return column_list;
}

/** Look for a transition from land to sea.

\param s A pointer to a Sed_cube.
\param n The row or column to look along.
\param vary_dim The dimension along which to look.

\return The indices to the land column that marks the transition.
*/
Eh_ind_2 *sed_cube_find_shore( Sed_cube s , int n , int vary_dim )
{
   int i, j;
   int row, col;
   Eh_ind_2 *pos=NULL;

   if ( vary_dim==VARY_COLS )
   {
      row = n;
      for ( j=0 ; j<s->n_y-1 && !pos ; j++ )
         if (   sed_column_is_below( s->col[row][j]   , s->sea_level )
              ^ sed_column_is_below( s->col[row][j+1] , s->sea_level ) )
         {
            pos = eh_new( Eh_ind_2 , 1 );
            if ( sed_column_is_below( s->col[row][j] , s->sea_level ) )
               pos->j = j+1;
            else
               pos->j = j;
            pos->i = row;
         }
   }
   else
   {
      col = n;
      for ( i=0 ; i<s->n_x-1 && !pos ; i++ )
         if (   sed_column_is_below( s->col[i][col]   , s->sea_level )
              ^ sed_column_is_below( s->col[i+1][col] , s->sea_level ) )
         {
            pos = eh_new( Eh_ind_2 , 1 );
            if ( sed_column_is_below( s->col[i][col] , s->sea_level ) )
               pos->i = i+1;
            else
               pos->i = i;
            pos->j = col;
         }
   }

   return pos;
}

GTree *sed_create_shore_tree( GList *shore )
{
   GTree *tree = g_tree_new( (GCompareFunc)&eh_compare_ind_2 );
   while ( shore )
   {
      g_tree_insert( tree , shore->data , shore->data );
      shore = shore->next;
   }
   return tree;
}

Sed_river *sed_cube_find_river_mouth( Sed_cube c , Sed_river *this_river )
{
   Eh_ind_2 hinge_pos;
   Eh_ind_2 *new_pos;
   double hinge_angle;

   hinge_pos     = eh_ind_2_create( this_river->hinge->x ,
                                    this_river->hinge->y );
   hinge_angle   = this_river->hinge->angle;

   new_pos       = sed_find_river_mouth( c , &hinge_pos , hinge_angle );

   this_river->x_ind = new_pos->i;
   this_river->y_ind = new_pos->j;

   eh_free( new_pos );

   return this_river;
}

GList *sed_cube_find_river_path( Sed_cube c          ,
                                 Eh_ind_2 *hinge_pos ,
                                 double angle )
{
   GList *river = sed_cube_find_line_path( c , hinge_pos , angle );
   return river;
}

Eh_ind_2 *sed_find_river_mouth( Sed_cube c          ,
                                Eh_ind_2 *hinge_pos ,
                                double angle )
{
   GList *this_link;
   Eh_ind_2 *ans;
   GList *river = sed_cube_find_river_path( c , hinge_pos , angle );

   ans = eh_ind_2_dup( river->data , NULL );

   for ( this_link=river ; this_link ; this_link=this_link->next )
      eh_free( this_link->data );

   g_list_free( river );

   return ans;
}

Eh_ind_2 get_offset_from_angle( double angle , double aspect )
{
   double shift_x, shift_y;
   double angle_diag = atan( aspect );

   eh_require( aspect>0 );

   if ( fabs(sin( angle )) - fabs(sin( angle_diag )) > -1e-5 )
      shift_y = (sin(angle)>0)?1:-1;
   else
      shift_y = 0;

   if ( fabs(cos( angle )) - fabs(cos( angle_diag )) > -1e-5 ) 
      shift_x = (cos(angle)>0)?1:-1;
   else
      shift_x = 0;

   eh_require( shift_x || shift_y );

   return eh_ind_2_create( shift_x , shift_y );
}

int get_path_exit_side( Eh_pt_2 pos_in_box , double angle , double dx , double dy )
{
   int i;
   Eh_pt_2 corner_pos[4] = { {0,0} , {1,0} , {1,1} , {0,1} };
   double angle_to_corner=-M_PI;
   double rise, run;

   eh_require( angle>=-M_PI );
   eh_require( angle<= M_PI );

   for ( i=0 ; i<4 && angle_to_corner<angle ; i++ )
   {
      rise = corner_pos[i].y*dy - pos_in_box.y;
      run  = corner_pos[i].x*dx - pos_in_box.x;

      if ( rise==0 && run<0 )
         angle_to_corner = (i==0)?-M_PI:M_PI;
      else
         angle_to_corner = atan2( rise , run );
   }

   if ( angle_to_corner<angle )
      i = 0;
   else
      i--;

   return i;
}

Eh_pt_2 get_path_exit_pos( Eh_pt_2 pos_in_box ,
                           double angle       ,
                           double dx          ,
                           double dy )
{
   int side;
   Eh_pt_2 exit_pos;

   side = get_path_exit_side( pos_in_box , angle , dx , dy );

   if ( side == 0 || side == 2 )
   {
      exit_pos.x = (side==0)?0:dx;
      if ( side==0 )
         exit_pos.y = -tan( angle )*pos_in_box.x+pos_in_box.y;
      else
         exit_pos.y = tan( angle )*(dx-pos_in_box.x)+pos_in_box.y;
   }
   else
   {
      if ( side==1 )
         exit_pos.x = -pos_in_box.y/tan( angle )+pos_in_box.x;
      else
         exit_pos.x = (dy-pos_in_box.y)/tan( angle )+pos_in_box.x;
      exit_pos.y = (side==1)?0:dy;
   }
/*
eh_watch_dbl( exit_pos.x );
eh_watch_dbl( exit_pos.y );
*/

   return exit_pos;
}

Eh_pt_2 get_path_entrance_pos( Eh_pt_2 exit_pos , double dx , double dy )
{
   Eh_pt_2 entrance_pos = exit_pos;

   if ( fabs(exit_pos.x) < 1e-5 )
      entrance_pos.x = dx;
   else if ( fabs(exit_pos.x-dx) < 1e-5 )
      entrance_pos.x = 0;

   if ( fabs(exit_pos.y) < 1e-5 )
      entrance_pos.y = dy;
   else if ( fabs(exit_pos.y-dy) < 1e-5 )
      entrance_pos.y = 0;

   return entrance_pos;
}

Eh_ind_2 get_shift_from_exit_pos( Eh_pt_2 exit_pos , double dx , double dy )
{
   Eh_ind_2 shift = { 0 , 0 };

   if ( fabs(exit_pos.x) < 1e-5 )
      shift.i = -1;
   else if ( fabs(exit_pos.x-dx) < 1e-5 )
      shift.i = 1;

   if ( fabs(exit_pos.y) < 1e-5 )
      shift.j = -1;
   else if ( fabs(exit_pos.y-dy) < 1e-5 )
      shift.j = 1;

   return shift;
}

gssize *sed_cube_river_path_id( Sed_cube c   ,
                                GList *river ,
                                gboolean down_stream )
{
   Eh_ind_2 hinge = eh_ind_2_create( ((Sed_river*)(river->data))->hinge->x ,
                                     ((Sed_river*)(river->data))->hinge->y );
   double angle = sed_get_river_angle( river->data );
   GList *path, *this_link;
   gssize i, n_y;
   gssize *path_id;

   path = sed_cube_find_line_path( c , &hinge , angle );
   n_y = g_list_length( path );
   path_id = eh_new( gssize , n_y+1 );

   if ( down_stream )
      path = g_list_reverse( path );

   for ( i=0,this_link=path ; this_link ; this_link=this_link->next,i++ )
   {
      path_id[i] = eh_grid_sub_to_id( c->n_y ,
                                      ((Eh_ind_2*)(this_link->data))->i ,
                                      ((Eh_ind_2*)(this_link->data))->j );
      eh_free( this_link->data );
   }
   path_id[n_y] = -1;

   g_list_free( path );

   return path_id;
}

GList *sed_cube_river_path( Sed_cube c , GList *river )
{
   Eh_ind_2 hinge = eh_ind_2_create( ((Sed_river*)(river->data))->hinge->x ,
                                     ((Sed_river*)(river->data))->hinge->y );
   double angle = sed_get_river_angle( river->data );

   return sed_cube_find_line_path( c , &hinge , angle );
}

GList *sed_cube_find_line_path( Sed_cube c          ,
                                Eh_ind_2 *hinge_pos ,
                                double angle )
{
   int i, j;
   GList *river_path=NULL;
   Eh_ind_2 river_pos;
   Eh_ind_2 shift;
   Eh_pt_2 pos_in_cell;

   angle = eh_reduce_angle( angle );

   pos_in_cell.x = .5*c->dx;
   pos_in_cell.y = .5*c->dy;

   eh_require( c );
   eh_require( hinge_pos );
   eh_require( is_in_domain( c->n_x , c->n_y , hinge_pos->i , hinge_pos->j ) );

   i = hinge_pos->i;
   j = hinge_pos->j;

   river_path = g_list_prepend( river_path , eh_ind_2_dup( hinge_pos , NULL ) );

   if ( sed_column_is_below( c->col[i][j] , c->sea_level ) )
      return river_path;

   while (     is_in_domain( c->n_x , c->n_y , i , j )
           &&  sed_column_is_above( c->col[i][j] , c->sea_level-1e-3 ) )
   {
      pos_in_cell = get_path_exit_pos( pos_in_cell , angle , c->dx , c->dy );

      shift = get_shift_from_exit_pos( pos_in_cell , c->dx , c->dy );
      pos_in_cell = get_path_entrance_pos( pos_in_cell , c->dx , c->dy );

      i += shift.i;
      j += shift.j;

      if ( is_in_domain( c->n_x , c->n_y , i , j ) )
      {
         river_pos = eh_ind_2_create( i , j );

         river_path = g_list_prepend( river_path ,
                                      eh_ind_2_dup( &river_pos , NULL ) );
      }

   }

   return river_path;
}

int is_river_mouth( Eh_ind_2 *shore_pos , Sed_hinge_pt* dir )
{
   int river_x, river_y;
   double hinge_ang = dir->angle;
   int    hinge_x   = dir->x;
   int    hinge_y   = dir->y;

   river_y = shore_pos->j;

   river_x = hinge_x + (river_y - hinge_y) / tan( hinge_ang );

   if ( river_x<shore_pos->i )
      return -1;
   else if ( river_x>shore_pos->i )
      return 1;
   else
      return 0;
}

Sed_river *sed_create_river( int n_grains , Eh_ind_2 *pos )
{
   Sed_river *river = eh_new( Sed_river , 1 );

   river->data = sed_hydro_new( n_grains );
   river->hinge = eh_new( Sed_hinge_pt , 1 );
   river->hinge->min_angle = 0;
   river->hinge->max_angle = M_PI;

   if ( pos )
   {
      river->x_ind = pos->i;
      river->y_ind = pos->j;
   }
   else
   {
      river->x_ind = 0;
      river->y_ind = 0;
   }

   return river;
}

Sed_river *sed_copy_river( Sed_river *dest , Sed_river *source )
{
   if ( !dest )
      dest = eh_new( Sed_river , 1 );

   memcpy( dest , source , sizeof( Sed_river ) );

   dest->data  = sed_hydro_dup( source->data );
   dest->hinge = eh_new( Sed_hinge_pt , 1 );

   memcpy( dest->hinge , source->hinge , sizeof( Sed_hinge_pt ) );

   return dest;
}

Sed_river *sed_dup_river( Sed_river *source )
{
   return sed_copy_river( NULL , source );
}

void sed_dump_river( FILE *fp , Sed_river *river )
{
   int len = strlen( river->river_name );

   sed_hydro_write( fp , river->data );

   fwrite( river->hinge      , sizeof(Sed_hinge_pt) , 1   , fp );
   fwrite( &(river->x_ind)   , sizeof(int)          , 1   , fp );
   fwrite( &(river->y_ind)   , sizeof(int)          , 1   , fp );
   fwrite( &len              , sizeof(int)          , 1   , fp );
   fwrite( river->river_name , sizeof(char)         , len , fp );
}

Sed_river *sed_load_river( FILE *fp )
{
   int len;
   Sed_river *river = eh_new( Sed_river , 1 );

   river->data = sed_hydro_read( fp );
   fread( &(river->x_ind)   , sizeof(int)          , 1   , fp );
   fread( &(river->y_ind)   , sizeof(int)          , 1   , fp );
   fread( &len              , sizeof(int)          , 1   , fp );
   fread( river->river_name , sizeof(char)         , len , fp );

   return river;
}

void sed_destroy_river( Sed_river *river )
{
   if ( river )
   {
      eh_free( river->hinge );
      sed_hydro_destroy( river->data );
      eh_free( river );
   }
}

double sed_get_river_angle( Sed_river *river )
{
   return river->hinge->angle;
}

double sed_cube_river_angle( Sed_cube c , GList *river )
{
   return sed_get_river_angle( (Sed_river*)(river->data) );
}

Eh_dbl_grid sed_get_floor_3_default( int floor_type , int n_x , int n_y )
{
   int i, j;
   int dy = 100;
   double m = .02;
   Eh_dbl_grid grid;

   if ( n_x <= 0 )
      n_x = 100;
   if ( n_y <= 0 )
      n_y = 100;

   grid = eh_grid_new( double , n_x , n_y );

   switch ( floor_type )
   {
      case 1:
         for ( i=0 ; i<n_x ; i++ )
            for ( j=0 ; j<n_y ; j++ )
               eh_dbl_grid_set_val( grid , i , j , -m*j*dy );
         break;
      case 2:
         for ( i=0 ; i<n_x ; i++ )
            for ( j=0 ; j<n_y ; j++ )
               if ( j<n_y/2 )
                  eh_dbl_grid_set_val( grid , i , j , -m*j*dy );
               else
                  eh_dbl_grid_set_val( grid , i , j , -2*m*j*dy );
         break;
      default:
         for ( i=0 ; i<n_x ; i++ )
            for ( j=0 ; j<n_y ; j++ )
               eh_dbl_grid_set_val( grid , i , j , -1 );
         break;
   }

   return grid;
}

Eh_dbl_grid
sed_get_floor_1d_grid( const char *file , double dx , double dy , GError **err )
{
   Eh_dbl_grid grid = NULL;

   eh_return_val_if_fail( err==NULL || *err==NULL , NULL );

   //---
   // Scan the data file.  The should only be one record.  If there
   // are more, we'll just ignore them.
   //---
   eh_debug( "Scan the bathymetry file" );
   {
      double** data;
      GError* tmp_err = NULL;
      gint n_rows, n_cols;

      data = eh_dlm_read_swap( file , ";," , &n_rows , &n_cols , &tmp_err );

      if ( tmp_err )
         g_propagate_error( err , tmp_err );
      else if ( n_rows!=2 )
         g_set_error( &tmp_err ,
            SED_CUBE_ERROR ,
            SED_CUBE_ERROR_NOT_TWO_COLUMNS ,
            "%s: Bathymetry file does not contain 2 columns (found %d)\n" ,
            file , n_rows );
      else if ( n_cols<2 )
         g_set_error( &tmp_err ,
            SED_CUBE_ERROR ,
            SED_CUBE_ERROR_INSUFFICIENT_DATA ,
            "%s: Bathymetry file contains only one data point\n" ,
            file );
      else if ( !eh_dbl_array_is_monotonic_up( data[0] , n_cols ) )
         g_set_error( &tmp_err ,
            SED_CUBE_ERROR ,
            SED_CUBE_ERROR_TIME_NOT_MONOTONIC ,
            "%s: The position data must be monotonically increasing.\n" ,
            file );
      else if ( data[0][0]>0 )
         g_set_error( &tmp_err ,
            SED_CUBE_ERROR ,
            SED_CUBE_ERROR_DATA_BAD_RANGE ,
            "%s: Insufficient range in position data.\n" ,
            file );
      else
      {
         double *y, *z;
         gint n;

         //---
         // The cross-shore positions will be the first row, and the depths the
         // second row (in the file they are listed in columns).
         // They should both be the same length.  We don't bother checking.
         //---
         y = data[0];
         z = data[1];
         n = n_cols;

         //---
         // Interpolate the data from the file to an equally spaced grid.
         //---
         eh_debug( "Interpolate to a uniform grid" );
         {
            gssize n_y = (y[n-1]-y[0]) / dy;
            grid = eh_grid_new( double , 1 , n_y );
            eh_grid_set_y_lin( grid , y[0] , dy );

            interpolate( y , z , n , eh_grid_y(grid) , eh_grid_data_start(grid) , n_y );
         }

      }

      eh_free_2( data );

      if ( tmp_err!=NULL )
         g_propagate_error( err , tmp_err );

      eh_debug( "done." );
   }


   return grid;
}

#define S_KEY_SUBSIDENCE_TIME "time"

Eh_sequence *sed_get_floor_sequence_2( const char *file ,
                                       double *y_i      ,
                                       gssize n_yi      ,
                                       GError** error )
{
   Eh_sequence* grid_seq = NULL;
   gint*        n_rows   = NULL;
   gint*        n_cols   = NULL;
   gchar**      data     = NULL;
   GError*      err      = NULL;
   double***    all_records;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   all_records = eh_dlm_read_full_swap( file  , "," , &n_rows , &n_cols ,
                                        &data , -1  , &err );

   if ( all_records )
   {
      gint i;
      gint n_recs = g_strv_length( (gchar**)all_records );
      Eh_symbol_table tab;
      double* t = eh_new( double , n_recs );

      /* Read the time keys */
      for ( i=0 ; i<n_recs && err==NULL ; i++ )
      {
         tab = eh_str_parse_key_value( data[i] , ":" , "\n" );

         if ( tab && eh_symbol_table_has_label( tab , S_KEY_SUBSIDENCE_TIME ) )
            t[i] = eh_symbol_table_dbl_value( tab , S_KEY_SUBSIDENCE_TIME );
         else
         {
            g_set_error( &err ,
                         SED_CUBE_ERROR ,
                         SED_CUBE_ERROR_NO_TIME_LABEL , 
                         "Time label not found "
                         "for record %d in %s" , i+1 , file );
         }

         tab = eh_symbol_table_destroy( tab );
      }

      /* and make sure they're monotonically increasing */
      if ( err==NULL && !eh_dbl_array_is_monotonic_up(t,n_recs ) )
      {
         g_set_error( &err ,
            SED_CUBE_ERROR ,
            SED_CUBE_ERROR_TIME_NOT_MONOTONIC ,
            "%s: The grid sequence must be monotonically increasing.\n" ,
            file );
      }

      for ( i=0 ; i<n_recs && err==NULL ; i++ )
      {
         /* Make sure there are only two columns of data in the file */
         if ( n_rows[i]!=2 )
         {
            g_set_error( &err ,
               SED_CUBE_ERROR ,
               SED_CUBE_ERROR_NOT_TWO_COLUMNS ,
               "%s: Record number %d does not contain 2 columns (found %d)\n" ,
               file , i+1 , n_rows[i] );
         }
         /* and that there are at least two data points */
         else if ( n_cols[i]<2 )
         {
            g_set_error( &err ,
               SED_CUBE_ERROR ,
               SED_CUBE_ERROR_INSUFFICIENT_DATA ,
               "%s: Record number %d contains only one data point\n" ,
               file , i+1 );
         }
      }

      /* Read the data for each record of the sequence */
      if ( err==NULL )
      {
         Eh_dbl_grid     this_grid;
         double*         y;
         double*         z;

         grid_seq = eh_create_sequence( );

         for ( i=0 ; i<n_recs && err==NULL ; i++ )
         {
            y   = (all_records[i])[0];
            z   = (all_records[i])[1];

            if ( !eh_dbl_array_is_monotonic_up(y,n_cols[i] ) )
               g_set_error( &err ,
                  SED_CUBE_ERROR ,
                  SED_CUBE_ERROR_DATA_NOT_MONOTONIC ,
                  "%s (record %d): Position data not monotonically increasing.\n" ,
                  file , i+1 );
            else if ( y[0]>y_i[0] || y[n_cols[i]-1]<y_i[n_yi-1] )
               g_set_error( &err ,
                  SED_CUBE_ERROR ,
                  SED_CUBE_ERROR_DATA_BAD_RANGE ,
                  "%s (record %d): Insufficient range in position data.\n" ,
                  file , i+1 );
            else
            {
               this_grid = eh_grid_new( double , 1 , n_yi );
               g_memmove( eh_grid_y(this_grid) , y_i , n_yi*sizeof(double) );

               interpolate( y , z , n_cols[i] ,
                            eh_grid_y(this_grid) , eh_grid_data_start(this_grid) , eh_grid_n_y(this_grid) );

               eh_add_to_sequence( grid_seq , t[i] , this_grid );
            }
         }
      }

      eh_free( t );

      for ( i=0 ; i<n_recs ; i++ )
         eh_free_2( all_records[i] );
   }

   if ( err!=NULL )
      g_propagate_error( error , err );

   g_strfreev( data );
   eh_free( n_rows      );
   eh_free( n_cols      );
   eh_free( all_records );

   return grid_seq;
}

Eh_sequence *sed_get_floor_sequence_3( const char *file ,
                                       double dx        ,
                                       double dy )
{
   Eh_dbl_grid grid;
   Eh_sequence *grid_seq;
   FILE *fp;
   int i, n, n_elem;
   size_t start, end;
   gint n_x, n_y, n_t;
   double t;

   //---
   // Open the sequence file.
   //---
   fp = fopen( file , "rb" );
   if ( !fp )
      eh_error( "could not open file : %s" , file );

   //---
   // The number of columns and rows are listed first as an int (32).
   //---
   fread( &n_y , sizeof(gint32) , 1 , fp );
   fread( &n_x , sizeof(gint32) , 1 , fp );

   //---
   // Calculate the number of rows in the file based on the number of columns
   // and the file size.  If this is not a whole number, something is wrong.
   //---
   start = ftell( fp );
   fseek( fp , 0 , SEEK_END );
   end = ftell( fp );
   fseek( fp , start , SEEK_SET );

   n_elem = (end-start)/sizeof(double);
   n_t    = n_elem/(n_x*n_y+1);

   if ( (n_x*n_y+1)*n_t != n_elem )
   {
      eh_message( "%s: this file appears to have been truncated. " , file );
      eh_message( "%s: reading %d records. " , file , n_t );
      eh_message( "%s: discarding the final %d records. " ,
                  file , n_elem-n_t*(n_x*n_y+1) );
   }

   //---
   // Read the sequence of grids.  A frame of a sequence consists of a key
   // followed by a square grid.  Each key is a double and each grid is a
   // series of doubles.  All grids are the same size that is given by the
   // first to elements in the file as two (32 bit) ints.
   //---
   grid_seq = eh_create_sequence( );

   for ( n=0 ; n<n_t ;  n++ )
   {
      fread( &t , sizeof(double) , 1 , fp );

      grid = eh_grid_new( double , n_x , n_y );

      fread( eh_grid_data_start(grid) , sizeof(double) , n_x*n_y , fp );

      for ( i=0 ; i<n_x ; i++ )
         eh_grid_x(grid)[i] = i*dx;

      for ( i=0 ; i<n_y ; i++ )
         eh_grid_y(grid)[i] = i*dy;

      eh_add_to_sequence( grid_seq , t , grid );
   }

   //---
   // Ensure that the sequence is nonotonically increasing.
   //---
   for ( n=1 ; n<grid_seq->len ; n++ )
   {
      if ( !(grid_seq->t[n-1] < grid_seq->t[n]) )
      {
         eh_message( "The grid sequence must be monotonically increasing." );
         eh_message( "Frame %d has a key of %f." , n-1 , grid_seq->t[n-1]  );
         eh_message( "Frame %d has a key of %f." , n   , grid_seq->t[n]    );
         eh_error  ( "Error reading grid sequence from file: %s." , file   );
      }
   }

   fclose( fp );

   return grid_seq;
}

Eh_dbl_grid sed_get_floor_2d_grid( const char *file , double dx , double dy )
{
   Eh_dbl_grid grid;
   int i, n_elem;
   gint32 n_x, n_y;
   size_t start, end;
   FILE *fp;

   //---
   // Open the bathymetry file.
   //---
   fp = fopen( file , "rb" );
   if ( !fp )
      eh_error( "could not open file : %s" , file );

   //---
   // The number of columns is listed first as an int (32).
   //---
   fread( &n_y , sizeof(gint32) , 1 , fp );

   //---
   // Calculate the number of rows in the file based on the number of columns
   // and the file size.  If this is not a whole number, something is wrong.
   //---
   start = ftell( fp );
   fseek( fp , 0 , SEEK_END );
   end = ftell( fp );
   fseek( fp , start , SEEK_SET );

   n_elem = (end-start)/sizeof(double);
   n_x = n_elem/n_y;

   if ( n_x*n_y != n_elem )
   {
      eh_message( "%s: this file appears to have been truncated. " , file );
      eh_message( "%s: reading %d rows. " , file , n_x );
      eh_message( "%s: discarding the final %d elements. " ,
                  file , n_elem-n_x*n_y );
   }

   //---
   // Read the grid data.
   //---
   grid = eh_grid_new( double , n_x , n_y );

   fread( eh_grid_data_start(grid) , sizeof(double) , n_x*n_y , fp );

   for ( i=0 ; i<n_x ; i++ )
      eh_grid_x(grid)[i] = i*dx;

   for ( i=0 ; i<n_y ; i++ )
      eh_grid_y(grid)[i] = i*dy;

   fclose( fp );

   return grid;
}

void sed_avulse_river( Sed_river *river , Sed_cube c )
{
   double last_angle, angle;
   double min_angle, max_angle;
   double std_dev;
   static long int iseed[1];

   last_angle = river->hinge->angle;
   min_angle  = river->hinge->min_angle;
   max_angle  = river->hinge->max_angle;
   std_dev    = river->hinge->std_dev;

   do
   {
      angle = last_angle + eh_gasdev( iseed )*std_dev;
      if ( angle<min_angle )
         angle = min_angle+(min_angle-angle);
      if ( angle>max_angle )
         angle = max_angle-(angle-max_angle);
   }
   while ( angle < min_angle || angle > max_angle );

   river->hinge->angle = angle;

   river = sed_cube_find_river_mouth( c , river );
}

void avulse_river_helper( Sed_river *this_river , Sed_cube c );

Sed_cube sed_cube_avulse_all_rivers( Sed_cube c )
{
   g_list_foreach( c->river , (GFunc)&avulse_river_helper , c );
   return c;
}

void avulse_river_helper( Sed_river *this_river , Sed_cube c )
{
   sed_avulse_river( this_river , c );
}

void find_river_mouth_helper( Sed_river *this_river , Sed_cube c );

Sed_cube sed_cube_find_all_river_mouths( Sed_cube c )
{
   g_list_foreach( c->river , (GFunc)&find_river_mouth_helper , c );
   return c;
}

void find_river_mouth_helper( Sed_river *this_river , Sed_cube c )
{
   sed_cube_find_river_mouth( c , this_river );
}

/** Write a Sed_cube to a binary file.

\todo  Members storm_list, shore, and constants are not written.  Fix this.

\param fp    A pointer to an open file.
\param p     A pointer to a Sed_cube .

*/
gssize sed_cube_write( FILE *fp , const Sed_cube p )
{
   gssize n = 0;

   if ( p && fp )
   {
      gssize i;
      gssize len = sed_cube_size(p);
      GList *list;

      /* Write all of the scalar data */
      n += fwrite( &(p->age)          , sizeof(double) , 1 , fp );
      n += fwrite( &(p->time_step)    , sizeof(double) , 1 , fp );
      n += fwrite( &(p->storm_value)  , sizeof(double) , 1 , fp );
      n += fwrite( &(p->quake_value)  , sizeof(double) , 1 , fp );
      n += fwrite( &(p->tidal_range)  , sizeof(double) , 1 , fp );
      n += fwrite( &(p->tidal_period) , sizeof(double) , 1 , fp );
      n += fwrite( &(p->n_x)          , sizeof(gint)   , 1 , fp );
      n += fwrite( &(p->n_y)          , sizeof(gint)   , 1 , fp );
      n += fwrite( &(p->basinWidth)   , sizeof(double) , 1 , fp );
      n += fwrite( &(p->dx)           , sizeof(double) , 1 , fp );
      n += fwrite( &(p->dy)           , sizeof(double) , 1 , fp );
      n += fwrite( &(p->sea_level)    , sizeof(double) , 1 , fp );
      n += fwrite( &(p->cell_height)  , sizeof(double) , 1 , fp );

      //---
      // Now write all of the pointer data.
      //---
      len = strlen( p->name );
      n += fwrite( &len    , sizeof(gssize) , 1   , fp );
      n += fwrite( p->name , sizeof(char)   , len , fp );
      n += fwrite( p->wave , sizeof(double) , 3   , fp );

      for ( i=0 ; i<len ; i++ )
         n += sed_column_write( fp , p->col[0][i] );

      n += sed_cell_write( fp , p->erode  );
      n += sed_cell_write( fp , p->remove );

      len = g_list_length( p->river );
      n += fwrite( &len , sizeof(gssize) , 1 , fp );

      // Dump the river mouths.
      for ( list=p->river ; list ; list=list->next )
         sed_dump_river( fp , list->data );

      // Dump the suspension grids.
      for ( list=p->in_suspension ; list ; list=list->next )
         eh_grid_dump( fp , list->data );
   }
   
   return n;
}

/** Read a Sed_cube from a binary file.

\param fp A pointer to an open file.

\return A pointer to a newly created Sed_cube.
*/
Sed_cube sed_cube_read( FILE *fp )
{
   Sed_cube p = NULL;

   eh_require( fp )
   {
      gssize i, j;
      gssize len;
      GList *list;
      Sed_river *this_river;
      Eh_grid this_grid;

      //---
      // Load in the scalar information into the Sed_cube.
      //---
      NEW_OBJECT( Sed_cube , p );
      fread( p , sizeof(*p) , 1 , fp );

      //---
      // Allocate memory, and load in the vector information into the Sed_cube.
      //---
      fread( &len    , sizeof(gssize) , 1   , fp );
      fread( p->name , sizeof(char)   , len , fp );
      fread( p->wave , sizeof(double) , 3   , fp );

      p->col    = eh_new( Sed_column* , p->n_x        );
      p->col[0] = eh_new( Sed_column  , p->n_x*p->n_y );
      for ( i=1 ; i<p->n_x ; i++ )
         p->col[i] = p->col[i-1]+p->n_y;
      for ( i=0 ; i<p->n_x ; i++ )
         for ( j=0 ; j<p->n_y ; j++ )
            p->col[i][j] = sed_column_read( fp );

      p->erode  = sed_cell_read( fp );
      p->remove = sed_cell_read( fp );

      // read the river data.
      fread( &len , sizeof(int) , 1 , fp );
      for ( i=0 ; i<len ; i++ )
      {
         this_river = sed_load_river( fp );
         sed_cube_add_river( p , this_river );
      }

      for ( list=p->in_suspension ; list ; list=list->next )
      {
         this_grid = eh_grid_load( fp );
         eh_grid_copy( list->data , this_grid );
         eh_grid_destroy( this_grid , TRUE );
      }

      sed_cube_set_shore( p );
   }

   return p;
}

gssize sed_cube_column_id( const Sed_cube c , double x , double y )
{
   gssize i, j;
   gssize row_ind, col_ind;

   if (    y > sed_column_y_position( c->col[0][c->n_y-1] ) 
        || y < sed_column_y_position( c->col[0][0] )
        || x > sed_column_x_position( c->col[c->n_x-1][0] ) 
        || x < sed_column_x_position( c->col[0][0] ) )
      return -1;

   for ( j=0 ;
         j<c->n_y && sed_column_y_position( c->col[0][j] ) <= y ;
         j++ );

   col_ind = (j==0)?0:j-1;

   for ( i=0 ;
         i<c->n_x && sed_column_x_position( c->col[i][col_ind] ) <= x ;
         i++ );

   row_ind = (i==0)?0:i-1;

   return eh_grid_sub_to_id( c->n_y , row_ind , col_ind );
}

gssize *sed_cube_find_column_below( Sed_cube c , double z )
{
   gssize i, j, n;
   gssize *col_id;

   col_id = NULL;
   for ( i=0,n=0 ; i<c->n_x ; i++ )
      for ( j=0 ; j<c->n_y ; j++ )
         if ( sed_column_top_height(c->col[i][j]) < z )
         {
            col_id      = g_renew( gssize , col_id , ++n );
            col_id[n-1] = eh_grid_sub_to_id(c->n_y,i,j);
         }
   col_id        = g_renew( gssize , col_id , ++n );
   col_id[n-1] = -1;

   return col_id;
}

gssize *sed_cube_find_column_above( Sed_cube c , double z )
{
   gssize i, j, n;
   gssize *col_id;

   col_id = NULL;
   for ( i=0,n=0 ; i<c->n_x ; i++ )
      for ( j=0 ; j<c->n_y ; j++ )
      {
         if ( sed_column_top_height(c->col[i][j]) > z )
         {
            col_id      = g_renew( gssize , col_id , ++n );
            col_id[n-1] = eh_grid_sub_to_id(c->n_y,i,j);
         }
      }
   col_id        = g_renew( gssize , col_id , ++n );
   col_id[n-1] = -1;

   return col_id;
}
/** Read floor elevation data from a file into an array.

Read a file of elevation data and interpolate to specified x-positions.  The
output vector, y must be allocated enough size to hold the elevation data.

@param filename A name of an input file.
@param y        A pointer to an array of y-positions.
@param len      Number of positions (length of x, and y).
@param z        A pointer to an array of elevations.

@return The number of elevation data read.

@see sed_get_floor .
*/
int sed_get_floor_vec(char *filename, double *y, int len, double *z)
{
   gssize new_len = 0;

   eh_require( filename );
   eh_require( y        );
   eh_require( z        );
   eh_require( len>0    );

   {
      gssize i;
      GArray *y_array=g_array_new( FALSE , FALSE , sizeof(double) );
      GArray *z_array;

      //---
      // Create an array of y's to pass to sed_get_floor
      //---
      for ( i=0 ; i<len ; i++ )
         g_array_append_val( y_array , y[i] );

      //---
      // Get the depths as an array
      //---
      z_array = sed_get_floor( filename , y_array );

      new_len = z_array->len;

      //---
      // Copy the array values to the input locations
      //---
      for ( i=0 ; i<new_len ; i++ )
      {
         z[i] = g_array_index( z_array , double , i );
         y[i] = g_array_index( y_array , double , i );
      }

      g_array_free( y_array , TRUE );
      g_array_free( z_array , TRUE );
   }

   return new_len;
}

/** Read elevation data from a file into a GArray.

Read a file of elevation data and interpolate to specified x-positions.  This
differs from sed_get_floor_vec in that here the input x-positions and output
elevations are held in GArray's rather than standard arrays.

@param file     A name of an elevation file.
@param y_array  A GArray of y-positions.

@return A GArray of the elevation data.

@see sed_get_floor_vec .
*/
GArray *sed_get_floor( char *file , GArray *y_array )
{
   Eh_data_record floor_data = NULL;
   GArray *z_array = g_array_new(FALSE,TRUE,sizeof(double));

   //---
   // Scan the data file.
   // The should only be one record.  If there is more, we'll just ignore them.
   //---
   {
      gssize i;
      Eh_data_record* all_data = eh_data_record_scan_file( file , "," , EH_FAST_DIM_COL , FALSE );

      if ( all_data )
      {
         floor_data = all_data[0];
         for ( i=1 ; all_data[i] ; i++ )
            eh_data_record_destroy( all_data[i] );
         eh_free( all_data );
      }
      else
         eh_error( "Error reading bathymetry file, %s." , file );
   }

   if ( floor_data )
   {
      double *y, *z;
      gssize n;

      //---
      // The cross-shore positions will be the first row, and the depths the
      // second row (in the file they are listed in columns).
      // They should both be the same length.  We don't bother checking.
      //---
      y = eh_data_record_row ( floor_data , 0 );
      z = eh_data_record_row ( floor_data , 1 );
      n = eh_data_record_size( floor_data , 1 );

      //---
      // Make sure the cross-shore distances are monotonically increasing.
      //---
      if ( !eh_dbl_array_is_monotonic_up( y , n ) )
         eh_error( "The position data must be monotonically increasing." );


      //---
      // Interpolate the data from the file to an equally spaced grid.
      //---
      {
         gssize i;
         interpolate( y , z , n , (double*)y_array->data , (double*)z_array->data , y_array->len );
         for ( i=0 ; i<y_array->len ; i++ )
         {
            if ( eh_isnan(g_array_index(z_array,double,i)) )
            {
               g_array_remove_index( y_array , i );
               g_array_remove_index( z_array , i );
            }
         }
      }

      eh_data_record_destroy( floor_data );
   }

   return z_array;
}

/** Get index from subscripts for a Sed_cube

@param p   A Sed_cube
@param i   First (slow) subscript
@param j   Second (fast) subscript

@return    The id corresponding to (i,j)
*/
gssize sed_cube_id( Sed_cube p , gssize i , gssize j )
{
   return eh_grid_sub_to_id( p->n_y , i , j );
}

/** Get subscripts from an id for a Sed_cube

@param p  A Sed_cube
@param id A one-dimensional index

@return The (i,j) subscript corresponding to an id
*/
Eh_ind_2 sed_cube_sub( Sed_cube p , gssize id )
{
   return eh_grid_id_to_sub( p->n_y , id );
}

/** Is i-j subscript within the domain of a Sed_cube

@param p A Sed_cube
@param i i-subscript
@param j j-subscript

@return TRUE if the subscript is within the domain
*/
gboolean sed_cube_is_in_domain( Sed_cube p , gssize i , gssize j )
{
   return is_in_domain( p->n_x , p->n_y , i , j );
}

/** Is id within the domain of a Sed_cube

@param p  A Sed_cube
@param id One-dimensional id

@return TRUE if the id is within the domain
*/
gboolean sed_cube_is_in_domain_id( Sed_cube p , gssize id )
{
   eh_return_val_if_fail( p!=NULL , FALSE );
   return id>=0 && id<sed_cube_size(p);
}

gboolean sed_cube_is_1d( Sed_cube p )
{
   eh_return_val_if_fail( p!=NULL , FALSE );
   return p->n_x == 1;
}

gssize
sed_cube_fprint( FILE* fp , Sed_cube c )
{
   gssize n = 0;

   if ( c )
   {
      gssize i;
      double z;
      double min_z =  G_MAXDOUBLE;
      double max_z = -G_MAXDOUBLE;

      n += fprintf( fp , "[Cube Info]\n" );

      n += fprintf( fp , "Name          = %s\n" , sed_cube_name (c) );

      n += fprintf( fp , "x resolution  = %f\n" , sed_cube_x_res(c) );
      n += fprintf( fp , "y resolution  = %f\n" , sed_cube_y_res(c) );
      n += fprintf( fp , "z resolution  = %f\n" , sed_cube_z_res(c) );

      for ( i=0 ; i<sed_cube_size(c) ; i++ )
      {
         z = sed_cube_base_height(c,0,i);
         if ( z < min_z )
            min_z = z;
         if ( z > max_z )
            max_z = z;
      }

      n == fprintf( fp , "No. x-columns = %d\n" , sed_cube_n_x(c) );
      n == fprintf( fp , "No. y-columns = %d\n" , sed_cube_n_y(c) );

      n += fprintf( fp , "Max elevation = %f\n" , max_z );
      n += fprintf( fp , "Min elevation = %f\n" , min_z );

      n += fprintf( fp , "Start         = %f\n" , sed_cube_col_y( c , 0 ) );
      n += fprintf( fp , "End           = %f\n" , sed_cube_col_y( c , sed_cube_n_y(c)-1 ) );

   }

   return n;
}

