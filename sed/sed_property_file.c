#include <stdio.h>
#include <glib.h>

#include "utils/utils.h"
#include "sed_property_file.h"

CLASS ( Sed_property_file_attr )
{
   Sed_get_val_func get_val;
   double lower_clip;
   double upper_clip;
   double water_val;
   double rock_val;
   double x_res;
   double y_res;
   double z_res;
   double x_lim[2];
   double y_lim[2];
   double z_lim[2];
   Sed_data_type type;
   gpointer user_data;
};

CLASS ( Sed_property_file_header )
{
   gssize n_rows;
   gssize n_x_cols;
   gssize n_y_cols;
   double cell_dx;
   double cell_dy;
   double cell_dz;
   double min_value, max_value;
   double water_value, rock_value;
   Sed_property property;
   double sea_level;
   double ref_x, ref_y, ref_z;
   int byte_order;
   int element_size;
};

CLASS ( Sed_property_file )
{
   FILE*                    fp;
   Sed_property             p;
   Sed_property_file_header h;
   Sed_property_file_attr   attr;
   double**                 data;
};

Sed_property_file sed_property_file_new( const char* file , Sed_property p , Sed_property_file_attr a )
{
   Sed_property_file f = NULL;

   eh_require( file );
   eh_require( p    );

   if ( file && p )
   {
      NEW_OBJECT( Sed_property_file , f );
      
      f->fp = fopen( file , "wb" );
      if ( !f->fp )
         eh_error( "Cound not open sedflux property file." );

      f->h = NULL;
      f->p = p;

      if ( a )
         f->attr = sed_property_file_attr_dup( a );
      else
         f->attr = sed_property_file_attr_new( );

      f->attr->get_val = NULL;

      f->data = NULL;
   }

   return f;
}

Sed_property_file sed_property_file_destroy( Sed_property_file f )
{
   if ( f )
   {
      if ( f->data )
      {
         eh_free( f->data[0] );
         eh_free( f->data );
      }

      sed_property_file_attr_destroy( f->attr );
      sed_property_file_header_destroy( f->h );

      fclose( f->fp );
      eh_free( f );
   }
   return NULL;
}

Sed_property_file_attr sed_property_file_attr_new( )
{
   Sed_property_file_attr a;

   NEW_OBJECT( Sed_property_file_attr , a );

   a->get_val = NULL;
   a->lower_clip = -G_MAXDOUBLE;
   a->upper_clip =  G_MAXDOUBLE;
   a->water_val  = -G_MAXDOUBLE;
   a->rock_val   =  G_MAXDOUBLE;
   a->x_res      =  0;
   a->y_res      =  0;
   a->z_res      =  0;
   a->x_lim[0]   = -G_MAXDOUBLE;
   a->x_lim[1]   =  G_MAXDOUBLE;
   a->y_lim[0]   = -G_MAXDOUBLE;
   a->y_lim[1]   =  G_MAXDOUBLE;
   a->z_lim[0]   = -G_MAXDOUBLE;
   a->z_lim[1]   =  G_MAXDOUBLE;
   a->type       = SED_TYPE_UINT8;
   a->user_data  = NULL;

   return a;
}

Sed_property_file_attr sed_property_file_attr_copy( Sed_property_file_attr dest , Sed_property_file_attr src )
{
   if ( !dest )
      dest = sed_property_file_attr_new();

   g_memmove( dest , src , sizeof( Sed_property_file_attr ) );

   return dest;
}

Sed_property_file_attr sed_property_file_attr_dup( Sed_property_file_attr src )
{
   return sed_property_file_attr_copy( NULL , src );
}

Sed_property_file_attr sed_property_file_attr_destroy( Sed_property_file_attr a )
{
   if ( a )
   {
      eh_free( a );
   }
   return NULL;
}

Sed_property_file_header sed_property_file_header_destroy( Sed_property_file_header h )
{
   if ( h )
   {
      sed_property_destroy( h->property );
      eh_free( h );
   }
   return NULL;
}

gssize sed_property_file_header_fprint( FILE *fp , Sed_property_file_header hdr );
Sed_property_file_header sed_property_file_header_new( const Sed_cube p ,
                                                       Eh_ndgrid g      ,
                                                       Sed_property property );
double sed_cube_min_height( Sed_cube p , gssize **col_id );
double sed_cube_max_height( Sed_cube p , gssize **col_id );
gssize *sed_cube_x_cols_between( Sed_cube p , double dx , double left , double right );
gssize *sed_cube_y_cols_between( Sed_cube p , double dy , double bottom , double top );
gssize sed_cube_n_rows_between( Sed_cube p , double dz , double lower , double upper , gssize *col_id );
Eh_ndgrid sed_cube_property_subgrid( Sed_cube p            ,
                                     Sed_property property ,
                                     double lower_left[3]  ,
                                     double upper_right[3] ,
                                     double resolution[3] );

gssize sed_property_file_write( Sed_property_file sed_fp , Sed_cube p )
{
   gssize n = 0;

   eh_require( sed_fp );
   eh_require( p      );

   if ( sed_fp && p )
   {
      Eh_ndgrid g;
      double    lower_left[3];
      double    upper_right[3];
      double    resolution[3];

      lower_left[0]  = sed_fp->attr->x_lim[0];
      lower_left[1]  = sed_fp->attr->y_lim[0];
      lower_left[2]  = sed_fp->attr->z_lim[0];
      upper_right[0] = sed_fp->attr->x_lim[1];
      upper_right[1] = sed_fp->attr->y_lim[1];
      upper_right[2] = sed_fp->attr->z_lim[1];
      resolution[0]  = sed_fp->attr->x_res;
      resolution[1]  = sed_fp->attr->y_res;
      resolution[2]  = sed_fp->attr->z_res;

      g = sed_cube_property_subgrid( p           ,
                                     sed_fp->p   ,
                                     lower_left  ,
                                     upper_right ,
                                     resolution );

      sed_fp->h = sed_property_file_header_new( p , g , sed_fp->p );

      n += sed_property_file_header_fprint( sed_fp->fp , sed_fp->h );
      n += eh_ndgrid_write( sed_fp->fp , g );

      eh_ndgrid_destroy( g );
   }

   return n;
}

gssize sed_property_file_header_fprint( FILE *fp , Sed_property_file_header hdr )
{
   gssize n = 0;

   eh_require( fp  );
   eh_require( hdr );

   if ( fp && hdr )
   {
      char* date_str    = eh_new( char , 2048 );
      char* program_str = eh_new( char , 2048 );
      char* property_name = sed_property_name( hdr->property );
      GDate *today = g_date_new( );

      g_date_set_time( today , time(NULL) );
      g_date_strftime( date_str , 2048 , "%A %e %B %Y %T %Z" , today );

      fflush( fp );

      n += fprintf( fp , "--- header ---\n" );

      g_snprintf( program_str , 2048 , "%s %s" ,
                  PROGRAM_NAME          ,
                  SED_VERSION_S );

      n += fprintf( fp , "SEDFLUX property file version: %s\n" , program_str );
      n += fprintf( fp , "Creation date: %s\n"                 , date_str );
      n += fprintf( fp , "Property: %s\n"                      , property_name );
      n += fprintf( fp , "Number of rows: %d\n"                , (gint)hdr->n_rows );
      n += fprintf( fp , "Number of x-columns: %d\n"           , (gint)hdr->n_x_cols );
      n += fprintf( fp , "Number of y-columns: %d\n"           , (gint)hdr->n_y_cols );
      n += fprintf( fp , "dx: %f\n"                            , hdr->cell_dx );
      n += fprintf( fp , "dy: %f\n"                            , hdr->cell_dy );
      n += fprintf( fp , "dz: %f\n"                            , hdr->cell_dz );
      n += fprintf( fp , "Sea level: %f\n"                     , hdr->sea_level );
      n += fprintf( fp , "Bottom-side coordinate: %f\n"        , hdr->ref_z );
      n += fprintf( fp , "North-side coordinate: %f\n"         , hdr->ref_x );
      n += fprintf( fp , "West-side coordinate: %f\n"          , hdr->ref_y );
      n += fprintf( fp , "Data type: %s\n"                     , "DOUBLE" );
      n += fprintf( fp , "Rock value: %g\n"                    , hdr->rock_value );
      n += fprintf( fp , "Water value: %g\n"                   , hdr->water_value );
      n += fprintf( fp , "Byte order: %d\n"                    , hdr->byte_order );

      n += fprintf( fp , "--- data ---\n" );

      fflush( fp );

      g_date_free( today );
      eh_free( property_name );
      eh_free( program_str );
      eh_free( date_str    );
   }

   return n;
}

Sed_property_file_header sed_property_file_header_new( const Sed_cube p ,
                                                       Eh_ndgrid g      ,
                                                       Sed_property property )
{
   Sed_property_file_header hdr = NULL;

   eh_require( p );
   eh_require( g );
   eh_require( property );

   if ( p && g && property )
   {
      NEW_OBJECT( Sed_property_file_header , hdr );

      hdr->n_rows       = eh_ndgrid_n( g , 2 );
      hdr->n_y_cols     = eh_ndgrid_n( g , 1 );
      hdr->n_x_cols     = eh_ndgrid_n( g , 0 );
      hdr->cell_dx      = sed_cube_x_res( p );
      hdr->cell_dy      = sed_cube_y_res( p );
      hdr->cell_dz      = sed_cube_z_res( p );
      hdr->property     = property;
      hdr->sea_level    = sed_cube_sea_level( p );
      hdr->ref_z        = eh_ndgrid_x( g , 2 )[0];
      hdr->ref_y        = eh_ndgrid_x( g , 1 )[0];
      hdr->ref_x        = eh_ndgrid_x( g , 0 )[0];
      hdr->byte_order   = G_BYTE_ORDER;
      hdr->element_size = sizeof(double);
      hdr->rock_value   =  G_MAXDOUBLE;
      hdr->water_value  = -G_MAXDOUBLE;
   }

   return hdr;
}

Eh_ndgrid sed_cube_property_subgrid( Sed_cube p            ,
                                     Sed_property property ,
                                     double lower_left[3]  ,
                                     double upper_right[3] ,
                                     double resolution[3] )
{
   gssize i, j, k, n, id;
   double bottom, top;
   double dx, dy, dz;
   double hydro_static;
   double *load;
   gssize sediment_rows, rock_rows, water_rows;
   gssize top_sed, bot_sed;
   Sed_column col_temp;
   Eh_dbl_grid g;
   Eh_ndgrid g_3;
   double lower_left_x, lower_left_y, lower_left_z;
   double upper_right_x, upper_right_y, upper_right_z;
   gssize *cols, *x_cols, *y_cols;
   gssize n_rows, n_x_cols, n_y_cols;
   gboolean with_load, excess_pressure;

   excess_pressure = sed_property_is_named( property , "EXCESS PRESSURE" );
   with_load = sed_property_is_named( property , "EXCESS PRESSURE" )
             | sed_property_is_named( property , "COHESION"        )
             | sed_property_is_named( property , "SHEAR STRENGTH"  );

   lower_left_x = sed_cube_col_x( p,0 );
   lower_left_y = sed_cube_col_y( p,0 );
   lower_left_z = sed_cube_min_height( p , NULL );

   if ( lower_left )
   {
      lower_left_x = eh_max( lower_left[0] , lower_left_x );
      lower_left_y = eh_max( lower_left[1] , lower_left_y );
      lower_left_z = eh_max( lower_left[2] , lower_left_z );
   }

   upper_right_x = sed_cube_col_x( p , sed_cube_size(p)-1 );
   upper_right_y = sed_cube_col_y( p , sed_cube_size(p)-1 );
   upper_right_z = sed_cube_max_height( p , NULL );

   if ( upper_right )
   {
      upper_right_x = eh_min( upper_right[0] , upper_right_x );
      upper_right_y = eh_min( upper_right[1] , upper_right_y );
      upper_right_z = eh_min( upper_right[2] , upper_right_z );
   }

   dx = sed_cube_x_res( p );
   dy = sed_cube_y_res( p );
   dz = sed_cube_z_res( p );

   if ( resolution )
   {
      dx = (resolution[0]>0)?resolution[0]:sed_cube_x_res( p );
      dy = (resolution[1]>0)?resolution[1]:sed_cube_y_res( p );
      dz = (resolution[2]>0)?resolution[2]:sed_cube_z_res( p );
   }

   x_cols = sed_cube_x_cols_between( p , dx , lower_left_x , upper_right_x );
   y_cols = sed_cube_y_cols_between( p , dy , lower_left_y , upper_right_y );

   for ( n_x_cols=0 ; x_cols[n_x_cols]>=0 ; n_x_cols++ );
   for ( n_y_cols=0 ; y_cols[n_y_cols]>=0 ; n_y_cols++ );

   cols = eh_new( gssize , n_x_cols*n_y_cols+1 );
   for ( i=0,n=0 ; i<n_x_cols ; i++ )
      for ( j=0 ; j<n_y_cols ; j++,n++ )
         cols[n] = sed_cube_id( p , x_cols[i] , y_cols[j] );
   cols[n] = -1;

   eh_free( x_cols );
   eh_free( y_cols );

   n_rows = sed_cube_n_rows_between( p , dz , lower_left_z , upper_right_z , cols );
   g_3    = eh_ndgrid_malloc( 3 , sizeof(double) , n_x_cols , n_y_cols , n_rows );
   g      = eh_ndgrid_to_grid( g_3 );

   eh_dbl_array_grid( eh_ndgrid_x(g_3,0) , eh_ndgrid_n(g_3,0) , lower_left_x , dx );
   eh_dbl_array_grid( eh_ndgrid_x(g_3,1) , eh_ndgrid_n(g_3,1) , lower_left_y , dy );
   eh_dbl_array_grid( eh_ndgrid_x(g_3,2) , eh_ndgrid_n(g_3,2) , lower_left_z , dz );

   col_temp = sed_column_dup( sed_cube_col(p,cols[0]) );

   top    = lower_left_z + n_rows*dz;
   bottom = lower_left_z;
   for ( i=0,id=cols[0],n=0 ; cols[n]>=0 ; i++,id=cols[++n] )
   {
      sed_column_copy( col_temp , sed_cube_col(p,id) );

      sed_column_set_z_res( col_temp , dz );
      sed_column_rebin( col_temp );
      sed_column_strip( col_temp , bottom , top );

      water_rows = eh_round( (   lower_left_z
                               + n_rows*dz
                               - sed_column_top_height( col_temp ) )
                             / dz ,
                             1. );

      if ( water_rows<0 )
         water_rows = 0;
      sediment_rows = sed_column_len( col_temp );
      rock_rows     = n_rows - sediment_rows - water_rows;
      top_sed       = sediment_rows-1;
      bot_sed       = 0;

      if ( rock_rows<0 )
      {
         rock_rows = 0;
         sediment_rows = n_rows - water_rows;
         bot_sed = top_sed - sediment_rows+1;
         if ( sediment_rows<=0 )
         {
            sediment_rows = 0;
            water_rows = n_rows;
            top_sed = -1; bot_sed = 0;
         }
      }

      if ( with_load )
         load = sed_column_load( col_temp , bot_sed , sediment_rows , NULL );

      if ( excess_pressure )
      {
         hydro_static = sed_column_water_pressure( col_temp );
         for ( j=top_sed ; j>=bot_sed ; j-- )
            load[j-bot_sed] = hydro_static;
      }

      for (j=0,k=0 ;j<water_rows;j++,k++)
         eh_dbl_grid_set_val( g , i , k , -G_MAXDOUBLE );

      for ( j=top_sed ; j>=bot_sed ; j--,k++)
         eh_dbl_grid_set_val( g , i , k , sed_property_measure( property ,
                                                                sed_column_nth_cell( col_temp , j ) ,
                                                                (with_load)?(load[j-bot_sed]):(-1) ) );

      for (j=0;j<rock_rows;j++,k++)
         eh_dbl_grid_set_val( g , i , k , G_MAXDOUBLE );

      if ( with_load )
         eh_free( load );
   }

   eh_free( cols );
   sed_column_destroy( col_temp );
   eh_grid_destroy( g , FALSE );

   return g_3;
}

double sed_cube_min_height( Sed_cube p , gssize **col_id )
{
   double z = G_MAXDOUBLE;

   eh_require( p )
   {
      gssize id, len = sed_cube_size(p);

      for ( id=0 ; id<len ; id++ )
         z = eh_min( z , sed_cube_base_height(p,0,id) );

      if ( col_id )
      {
         gssize n;

         *col_id = NULL;
         for ( id=0,n=0 ; id<len ; id++ )
            if ( eh_compare_dbl( sed_cube_base_height(p,0,id) , z , 1e-12 ) )
            {
               *col_id        = g_renew( gssize , *col_id , ++n );
               (*col_id)[n-1] = id;
            }
         *col_id        = g_renew( gssize , *col_id , ++n );
         (*col_id)[n-1] = -1;
      }
   }

   return z;
}

double sed_cube_max_height( Sed_cube p , gssize **col_id )
{
   double z = -G_MAXDOUBLE;

   if ( p )
   {
      gssize id, len = sed_cube_size(p);

      for ( id=0 ; id<len ; id++ )
         z = eh_max( z , sed_cube_top_height(p,0,id) );

      if ( col_id )
      {
         gssize n;

         *col_id = NULL;
         for ( id=0,n=0 ; id<len ; id++ )
            if ( eh_compare_dbl( sed_cube_top_height(p,0,id) , z , 1e-12 ) )
            {
               *col_id        = g_renew( gssize , *col_id , ++n );
               (*col_id)[n-1] = id;
            }
         *col_id        = g_renew( gssize , *col_id , ++n );
         (*col_id)[n-1] = -1;
      }
   }

   return z;
}

/** Get the x-indices to columns between two x-locations.

@param p     A Sed_cube
@param dx    x-spacing to use
@param left  x-position of startig point
@param right x-position of ending point

@return Array of x-indices (terminated with -1)
*/
gssize *sed_cube_x_cols_between( Sed_cube p , double dx , double left , double right )
{
   gssize *id=NULL;

   eh_return_val_if_fail( dx>=0 , NULL );

   eh_require( p )
   {
      gssize n=0;
      double x, y_0=sed_cube_col_y(p,0);
      Eh_ind_2 sub;

      eh_lower_bound( left  , sed_cube_col_x(p,0                 ) );
      eh_upper_bound( right , sed_cube_col_x(p,sed_cube_size(p)-1) );

      for ( x=left ; x<=right ; x+=dx )
      {
         id      = g_renew( gssize , id , ++n );
         id[n-1] = sed_cube_column_id( p , x , y_0 );
         sub     = sed_cube_sub( p , id[n-1] );
         id[n-1] = sub.i;
      }
      id      = g_renew( gssize , id , ++n );
      id[n-1] = -1;
   }

   return id;
}

/** Get the y-indices to columns between two y-locations.

@param p      A Sed_cube
@param dy     y-spacing to use
@param bottom y-position of startig point
@param top    y-position of ending point

@return Array of y-indices (terminated with -1)
*/
gssize *sed_cube_y_cols_between( Sed_cube p , double dy , double bottom , double top )
{
   gssize *id=NULL;

   eh_return_val_if_fail( dy>=0 , NULL );

   eh_require( p )
   {
      gssize n=0;
      double x_0=sed_cube_col_x(p,0), y;
      Eh_ind_2 sub;

      eh_lower_bound( bottom  , sed_cube_col_y( p,0                 ) );
      eh_upper_bound( top     , sed_cube_col_y( p,sed_cube_size(p)-1) );

      for ( y=bottom ; y<=top ; y+=dy )
      {
         id      = g_renew( gssize , id , ++n );
         id[n-1] = sed_cube_column_id( p , x_0 , y );
         sub     = sed_cube_sub( p , id[n-1] );
         id[n-1] = sub.j;
      }
      id      = g_renew( gssize , id , ++n );
      id[n-1] = -1;
   }

   return id;
}

gssize
sed_cube_n_rows_between( Sed_cube p , double dz , double lower , double upper , gssize *col_id )
{
   gssize n_rows = 0;

   eh_require( p )
   {
      gssize id, n;
      gssize row_0, row_1;
      gssize bottom_row    = G_MAXINT32;
      gssize top_row       = G_MININT32;
      double rows_per_cell = sed_cube_z_res(p)/dz;

      for ( id=col_id[0],n=0 ; col_id[n]>=0 ; id=col_id[++n] )
      {
         row_0 = (long)(sed_cube_base_height(p,0,id)/dz);
         row_1 = row_0 +    sed_column_len( sed_cube_col(p,id) )
                         * ( rows_per_cell ) + 1;
         eh_set_min( bottom_row , row_0 );
         eh_set_max( top_row    , row_1 );
      }

      eh_lower_bound( bottom_row , lower/dz );
      eh_upper_bound( top_row    , upper/dz );

      n_rows = top_row-bottom_row;
   }

   return n_rows;
}

