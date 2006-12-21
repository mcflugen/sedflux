#include <stdio.h>
#include <glib.h>

#include "sed_tripod.h"

CLASS ( Sed_measurement )
{
   char* name;
   Sed_tripod_func f;
};

typedef struct
{
   char* name;
   Sed_tripod_func f;
} Sed_measurement_static;


CLASS ( Sed_tripod_header )
{
   int byte_order;
   char *parameter;
   Sed_measurement x;
   gboolean from_river_mouth;
   gssize n_x_cols;
   gssize n_y_cols;
   gssize n_tripods;
   double bad_val;
   GArray *pos;
};

CLASS ( Sed_tripod_attr )
{
   Sed_tripod_func get_val;
   double lower_clip;
   double upper_clip;
   double bad_val;
   Sed_data_type type;
   gpointer user_data;
};

CLASS ( Sed_tripod )
{
   FILE* fp;
   Sed_measurement x;
   gboolean header_is_written;
   Sed_tripod_header h;
   Sed_tripod_attr attr;
};

static Sed_measurement_static all_measurements[17] = {
   { "SLOPE"        , &sed_measure_cube_slope } ,
   { "DEPTH"        , &sed_measure_cube_water_depth } ,
   { "ELEVATION"    , &sed_measure_cube_elevation } ,
   { "THICKNESS"    , &sed_measure_cube_thickness } ,
   { "GRAIN"        , &sed_measure_cube_grain_size } ,
   { "AGE"          , &sed_measure_cube_age } ,
   { "SAND"         , &sed_measure_cube_sand_fraction } ,
   { "SILT"         , &sed_measure_cube_silt_fraction } ,
   { "CLAY"         , &sed_measure_cube_clay_fraction } ,
   { "MUD"          , &sed_measure_cube_mud_fraction } ,
   { "FACIES"       , &sed_measure_cube_facies } ,
   { "DENSITY"      , &sed_measure_cube_density } ,
   { "POROSITY"     , &sed_measure_cube_porosity } ,
   { "PERMEABILITY" , &sed_measure_cube_permeability } ,
   { "BASEMENT"     , &sed_measure_cube_basement } ,
   { "RIVER MOUTH"  , &sed_measure_cube_river_mouth } ,
   { NULL , NULL }
};

Sed_tripod sed_tripod_new( const char* file , Sed_measurement x , Sed_tripod_attr attr )
{
   Sed_tripod t;

   NEW_OBJECT( Sed_tripod , t );

   t->fp = fopen( file , "wb" );
   if ( !t->fp )
      eh_error( "Could not open tripod file" );

   t->h                 = sed_tripod_header_new( x );
   t->x                 = sed_measurement_dup( x );
   t->header_is_written = FALSE;

   if ( attr )
      t->attr = sed_tripod_attr_dup( attr );
   else
      t->attr = sed_tripod_attr_new( );

   if ( t->attr->get_val == NULL )
      t->attr->get_val = x->f;

   return t;
}

Sed_tripod sed_tripod_destroy( Sed_tripod t )
{
   if ( t )
   {
      sed_tripod_attr_destroy( t->attr );
      sed_tripod_header_destroy( t->h );
      sed_measurement_destroy( t->x );
      fclose( t->fp );
      eh_free( t );
   }

   return t;
}

Sed_tripod_attr sed_tripod_attr_new( )
{
   Sed_tripod_attr a;

   NEW_OBJECT( Sed_tripod_attr , a );

   a->get_val    = NULL;
   a->lower_clip = -G_MAXDOUBLE;
   a->upper_clip =  G_MAXDOUBLE;
   a->bad_val    = -G_MAXDOUBLE;
   a->type       = SED_TYPE_DOUBLE;
   a->user_data  = NULL;

   return a;
}

Sed_tripod_attr sed_tripod_attr_copy( Sed_tripod_attr dest , Sed_tripod_attr src )
{
   eh_require( src );

   if ( src )
   {
      if ( !dest )
         dest = sed_tripod_attr_new();

      g_memmove( dest , src , sizeof( Sed_tripod_attr ) );
   }
   else
      dest = NULL;

   return dest;
}

Sed_tripod_attr sed_tripod_attr_dup( Sed_tripod_attr src )
{
   return sed_tripod_attr_copy( NULL , src );
}

double* sed_tripod_measure( Sed_tripod t , Sed_cube c , Eh_pt_2* pos , double* data , gssize len )
{
   eh_require( t );
   eh_require( c );

   if ( t && c )
   {

      if ( pos )
      {
         gssize i, i_measure, j_measure;
         double x_0 = sed_cube_col_x(c,0);
         double y_0 = sed_cube_col_y(c,0);

         for ( i=0 ; i<len ; i++ )
         {
            i_measure = (gssize)((pos[i].x - x_0)/sed_cube_x_res( c ));
            j_measure = (gssize)((pos[i].y - y_0)/sed_cube_y_res( c ));

            if ( !sed_cube_is_in_domain( c , i_measure , j_measure ) )
            {
               eh_message( "OUT OF DOMAIN" );
               data[i] = eh_nan();
            }
            else
               data[i] = (t->x->f)( c , i_measure , j_measure );
         }
      }
      else
      {
         gssize id;
         for ( id=0 ; id<len ; id++ )
            data[id] = (t->x->f)( c , 0 , id );
      }
   }
   else
      data = NULL;

   return data;

}

Sed_tripod_attr sed_tripod_attr_destroy( Sed_tripod_attr a )
{
   if ( a )
   {
      eh_free( a );
   }
   return NULL;
}

Sed_measurement sed_measurement_new( const char* name )
{
   Sed_measurement m = NULL;

   if ( name )
   {
      gssize i;
      gboolean found = FALSE;

      NEW_OBJECT( Sed_measurement , m );

      m->name = g_strdup( name );
      m->f    = NULL;

      for ( i=0 ; !found && all_measurements[i].name ; i++ )
         if ( g_ascii_strcasecmp( all_measurements[i].name , name )==0 )
         {
            m->f = all_measurements[i].f;
            found = TRUE;
         }
   }

   return m;
}

Sed_measurement sed_measurement_copy( Sed_measurement dest , Sed_measurement src )
{
   eh_require( src );

   if ( src )
   {
      if ( !dest )
         dest = sed_measurement_new( src->name );

      eh_free( dest->name );

      dest->name = g_strdup( src->name );
      dest->f    = src->f;
   }
   else
      dest = NULL;

   return dest;
}

Sed_measurement sed_measurement_dup( Sed_measurement src )
{
   return sed_measurement_copy( NULL , src );
}

Sed_measurement sed_measurement_destroy( Sed_measurement m )
{
   if ( m )
   {
      eh_free( m->name );
      eh_free( m );
   }
   return NULL;
}

char* sed_measurement_name( Sed_measurement m )
{
   char* name;

   if ( m )
      name = g_strdup( m->name );
   else
      name = NULL;

   return name;
}

double sed_measure_cube_slope( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) )
      return sed_cube_slope( p , i , j );
   else
      return eh_nan();
}

double sed_measure_cube_water_depth( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) )
      return sed_cube_water_depth( p , i , j );
   else
      return eh_nan();
}

double sed_measure_cube_elevation( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) )
      return sed_cube_top_height( p , i , j );
   else
      return eh_nan();
}

double sed_measure_cube_thickness( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) )
      return sed_cube_thickness( p , i , j );
   else
      return eh_nan();
}

double sed_measure_cube_basement( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) )
      return sed_cube_base_height( p , i , j );
   else
      return eh_nan();
}

double sed_measure_cube_grain_size( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) && sed_cube_thickness(p,i,j)>0 )
   {
      Sed_column col = sed_cube_col_ij( p,i,j );
      gssize i_top   = sed_column_top_index( col );

      return sed_cell_grain_size_in_phi( sed_column_nth_cell( col , i_top ) );
   }
   else
      return eh_nan();

}

double sed_measure_cube_age( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) && sed_cube_thickness(p,i,j)>0 )
   {
      Sed_column col = sed_cube_col_ij( p,i,j );
      gssize i_top   = sed_column_top_index( col );

      return sed_cell_age( sed_column_nth_cell( col , i_top ) );
   }
   else
      return eh_nan();
}

double sed_measure_cube_sand_fraction( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) && sed_cube_thickness(p,i,j)>0 )
   {
      Sed_column col = sed_cube_col_ij( p,i,j );
      gssize i_top   = sed_column_top_index( col );
      return sed_cell_size_class_percent( sed_column_nth_cell( col , i_top ) , S_SED_TYPE_SAND );
   }
   else
      return eh_nan();
}

double sed_measure_cube_silt_fraction( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) && sed_cube_thickness(p,i,j)>0 )
   {
      Sed_column col = sed_cube_col_ij( p,i,j );
      gssize i_top   = sed_column_top_index( col );
      return sed_cell_size_class_percent( sed_column_nth_cell( col , i_top ) , S_SED_TYPE_SILT );
   }
   else
      return eh_nan();
}

double sed_measure_cube_clay_fraction( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) && sed_cube_thickness(p,i,j)>0 )
   {
      Sed_column col = sed_cube_col_ij( p,i,j );
      gssize i_top   = sed_column_top_index( col );
      return sed_cell_size_class_percent( sed_column_nth_cell( col , i_top ) , S_SED_TYPE_CLAY );
   }
   else
      return eh_nan();
}

double sed_measure_cube_mud_fraction( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) && sed_cube_thickness(p,i,j)>0 )
   {
      Sed_column col = sed_cube_col_ij( p,i,j );
      gssize i_top   = sed_column_top_index( col );
      return   sed_cell_size_class_percent( sed_column_nth_cell( col , i_top ) , S_SED_TYPE_SILT )
             + sed_cell_size_class_percent( sed_column_nth_cell( col , i_top ) , S_SED_TYPE_CLAY );
   }
   else
      return eh_nan();
}

double sed_measure_cube_density( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) && sed_cube_thickness(p,i,j)>0 )
   {
      Sed_column col = sed_cube_col_ij( p,i,j );
      gssize i_top   = sed_column_top_index( col );
      return sed_cell_density( sed_column_nth_cell( col , i_top ) );
   }
   else
      return eh_nan();
}

double sed_measure_cube_porosity( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) && sed_cube_thickness(p,i,j)>0 )
   {
      Sed_column col = sed_cube_col_ij( p,i,j );
      gssize i_top   = sed_column_top_index( col );
      return sed_cell_porosity( sed_column_nth_cell( col , i_top ) );
   }
   else
      return eh_nan();
}

double sed_measure_cube_permeability( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) && sed_cube_thickness(p,i,j)>0 )
   {
      Sed_column col = sed_cube_col_ij( p,i,j );
      gssize i_top   = sed_column_top_index( col );
      return sed_cell_permeability( sed_column_nth_cell( col , i_top ) );
   }
   else
      return eh_nan();
}

double sed_measure_cube_facies( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) && sed_cube_thickness(p,i,j)>0 )
   {
      Sed_column col = sed_cube_col_ij( p,i,j );
      gssize i_top   = sed_column_top_index( col );
      return (double)sed_cell_facies( sed_column_nth_cell( col , i_top ) );
   }
   else
      return eh_nan();
}

double sed_measure_cube_river_mouth( Sed_cube p , gssize i , gssize j )
{
   if ( sed_cube_is_in_domain(p,i,j) )
   {
      gssize len = sed_cube_number_of_rivers( p );
      gssize n;
      Sed_river* this_river;
      gboolean found=FALSE;

      for ( n=0 ; n<len && !found ; n++ )
      {
         this_river = sed_cube_river( p , n );
         if ( this_river->x_ind==i && this_river->y_ind==j )
            found = TRUE;
      }

      return found?1:-1;
   }
   else
      return -1;
}

Sed_tripod_header sed_tripod_header_new( Sed_measurement x )
{
   Sed_tripod_header h;

   NEW_OBJECT( Sed_tripod_header , h );

   h->bad_val          = eh_nan();
   h->n_tripods        = 0;
   h->n_x_cols         = 0;
   h->n_y_cols         = 0;
   h->byte_order       = G_BYTE_ORDER;
   h->x                = sed_measurement_dup( x );
   h->from_river_mouth = FALSE;
   h->pos              = g_array_new( FALSE , FALSE , sizeof(Eh_pt_2) );
   h->parameter        = NULL;

   return h;
}

Sed_tripod_header sed_tripod_header_destroy( Sed_tripod_header h )
{
   if ( h )
   {
      eh_free( h->parameter );
      g_array_free( h->pos , TRUE );
      sed_measurement_destroy( h->x );
      eh_free( h );
   }
   return NULL;
}

gssize sed_tripod_header_fprint( FILE* fp , Sed_tripod_header h )
{
   gssize n = 0;

   eh_require( fp );
   eh_require( h  );
   
   if ( fp && h )
   {
      char *date_str    = eh_new( char , 2048 );
      char *program_str = eh_new( char , 2048 );
      GDate *today      = g_date_new( );
      GArray *pos;
      char *property_str;

      g_date_set_time( today , time(NULL) );
      g_date_strftime( date_str , 2048 , "%A %e %B %Y %T %Z" , today );
      property_str = sed_measurement_name( h->x );

      fflush( fp );

      n += fprintf( fp , "--- header ---\n" );

      g_snprintf( program_str , 2048 , "%s %s.%s.%s" ,
                  PROGRAM_NAME          ,
                  PROGRAM_MAJOR_VERSION ,
                  PROGRAM_MINOR_VERSION ,
                  PROGRAM_MICRO_VERSION );

      n += fprintf( fp , "SEDFLUX tripod file version: %s\n"   , program_str );
      n += fprintf( fp , "Creation date: %s\n"                 , date_str );
      n += fprintf( fp , "Property: %s\n"                      , property_str );
      n += fprintf( fp , "Number of tripods: %d\n"             , h->n_tripods );
      n += fprintf( fp , "Number of x-columns: %d\n"           , h->n_x_cols );
      n += fprintf( fp , "Number of y-columns: %d\n"           , h->n_y_cols );
      n += fprintf( fp , "Data type: %s\n"                     , "DOUBLE" );
      n += fprintf( fp , "No data value: %g\n"                 , eh_nan() );
      n += fprintf( fp , "Byte order: %d\n"                    , h->byte_order );

      pos = h->pos;
      n += fprintf( fp , "Origin : " );

      if ( h->from_river_mouth )
         n += fprintf( fp , "RIVER MOUTH\n" );
      else
         n += fprintf( fp , "GRID\n" );

      n += fprintf( fp , "--- data ---\n" );

      fflush( fp );

      g_date_free( today );
      eh_free( program_str );
      eh_free( date_str    );
      eh_free( property_str );
   }

   return n;
}

gssize sed_tripod_write( Sed_tripod t , Sed_cube cube )
{
   gssize n = 0;

   eh_require( t );
   eh_require( cube );

   if ( t && cube )
   {
      gssize i;
      double time;
      double *measurement;
      Eh_pt_2 *location;
      int n_measurements;
      gint32 rec_len;
      Sed_tripod_header h = t->h;

      //---
      // Set up the positions where a measurement will be made.  The positions
      // are given in the pos array of the header.  If the pos array is empty
      // (its length is <= 0), it means make a measurement at every column
      // in the cube.
      //---
      if ( h->pos->len > 0 )
         n_measurements = h->pos->len;
      else
         n_measurements = sed_cube_size( cube );

      measurement = eh_new( double  , n_measurements );
      location    = eh_new( Eh_pt_2 , n_measurements );

      if ( h->pos->len > 0 )
         for ( i=0 ; i<n_measurements ; i++ )
            location[i] = g_array_index( h->pos , Eh_pt_2 , i );
      else
         for ( i=0 ; i<n_measurements ; i++ )
         {
            location[i].x = sed_cube_col_x( cube , i );
            location[i].y = sed_cube_col_y( cube , i );
         }


      //---
      // Make the measurement.
      //---
      sed_tripod_measure( t           ,
                          cube        ,
                          location    ,
                          measurement ,
                          n_measurements );
   
      //---
      // Write the header if it hasn't been written yet.  The header_is_written
      // member can be set to true upon the creation of the measurement file if
      // you would rather just have the data and no header.
      //---
      if ( !t->header_is_written )
      {
         n += sed_tripod_header_fprint( t->fp , t->h );
         rec_len = 3*n_measurements + 1;
         n += fwrite( &rec_len , sizeof(gint32) , 1 , t->fp );
         t->header_is_written = TRUE;
      }

      //---
      // Write the measurements.  First the time of the measurement is written,
      // then the x-y positions of the measurements, the the measuremnts
      // themselves.  The locations are written as x-y pairs of doubles.
      //---
      time = sed_cube_age_in_years( cube );

      n += fwrite( &time       , sizeof(double)  , 1              , t->fp );
      n += fwrite( location    , sizeof(Eh_pt_2) , n_measurements , t->fp );
      n += fwrite( measurement , sizeof(double)  , n_measurements , t->fp );

      fflush( t->fp );

      eh_free( measurement );
      eh_free( location    );
   }

   return n;
}

Sed_tripod sed_tripod_set_len( Sed_tripod t , gssize len )
{
   if ( t )
   {
      t->h->n_tripods = len;
   }

   return t;
}

Sed_tripod sed_tripod_set_n_x( Sed_tripod t , gssize n_x )
{
   if ( t )
   {
      t->h->n_x_cols = n_x;
   }

   return t;
}

Sed_tripod sed_tripod_set_n_y( Sed_tripod t , gssize n_y )
{
   if ( t )
   {
      t->h->n_y_cols = n_y;
   }

   return t;
}


