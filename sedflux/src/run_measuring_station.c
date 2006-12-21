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
#include <string.h>
#include <limits.h>

#ifndef NAME_MAX
# define NAME_MAX 255
#endif

#include "utils.h"
#include "sed_sedflux.h"
#include "measuring_station.h"
/*
int write_measurement_header( FILE *fp , 
                              Met_station_t *measuring_station_const );
*/

Sed_process_info run_met_station(gpointer ptr,Sed_cube prof)
{
   Met_station_t *data=(Met_station_t*)ptr;
   Sed_process_info info = SED_EMPTY_INFO;

   if ( prof == NULL )
   {
      if ( data->initialized )
      {
         sed_tripod_destroy( data->met_fp );
         data->initialized = FALSE;
      }
      return SED_EMPTY_INFO;
   }

   if ( !data->initialized )
   {
      data->met_fp = sed_tripod_new( data->filename  , data->parameter , NULL );

      if ( data->pos->len <= 0 )
         sed_tripod_set_len( data->met_fp , sed_cube_size(prof) );
      else
         sed_tripod_set_len( data->met_fp , data->pos->len );
      sed_tripod_set_n_x( data->met_fp , sed_cube_n_x(prof) );
      sed_tripod_set_n_y( data->met_fp , sed_cube_n_y(prof) );

      data->initialized = TRUE;
   }

   sed_tripod_write( data->met_fp , prof );

   return info;
}

#define S_KEY_PARAMETER  "parameter to measure"
#define S_KEY_WHENCE     "position wrt river mouth"
#define S_KEY_POSITION   "position of station"
#define S_KEY_FILENAME   "filename"

gboolean init_met_station(Eh_symbol_table symbol_table,gpointer ptr)
{
   Met_station_t *data=(Met_station_t*)ptr;
   double d_val;
   int i;
   char **position;

   if ( symbol_table == NULL )
   {
      if ( data->pos )
         g_array_free( data->pos , FALSE );
      sed_measurement_destroy( data->parameter );
      eh_free( data->parameter_str );
      eh_free( data->filename );
      data->initialized = FALSE;
      return TRUE;
   }
   
   data->parameter_str    = eh_symbol_table_value     ( symbol_table , S_KEY_PARAMETER );
   data->from_river_mouth = eh_symbol_table_bool_value( symbol_table , S_KEY_WHENCE    );

   data->parameter        = sed_measurement_new( data->parameter_str );

   if ( strcasecmp( eh_symbol_table_lookup( symbol_table ,
                                            S_KEY_POSITION ) ,
                    "all" ) == 0 )
      data->pos = g_array_new( FALSE , FALSE , sizeof(double) );
   else
   {
      data->pos = g_array_new( FALSE , FALSE , sizeof(double) );
      position  = g_strsplit ( eh_symbol_table_lookup( 
                                  symbol_table ,
                                  S_KEY_POSITION ) ,
                               "," , -1 );
      for ( i=0 ; position[i] ; i++ )
      {
         d_val = g_strtod( position[i] , NULL );
         g_array_append_val( data->pos , d_val );
      }
      g_strfreev(position);
   }

   data->filename = eh_symbol_table_value( symbol_table , S_KEY_FILENAME );

   if ( !eh_try_open( data->filename ) )
      eh_exit(-1);

   return TRUE;
}

gboolean dump_measuring_station_data( gpointer ptr , FILE *fp )
{
   Met_station_t *data = (Met_station_t*)ptr;
   gint64 len;

   fwrite( &(data->initialized)      , sizeof(gboolean) , 1 , fp );
   fwrite( &(data->from_river_mouth) , sizeof(gboolean) , 1 , fp );

   len = strlen( data->filename )+1;
   fwrite( &len , sizeof(gint64) , 1 , fp );
   fwrite( data->filename , sizeof(char) , len , fp );

   len = strlen( data->parameter_str )+1;
   fwrite( &len , sizeof(gint64) , 1 , fp );
   fwrite( data->parameter_str , sizeof(char) , len , fp );

   len = data->pos->len;
   fwrite( &len , sizeof(gint64) , 1 , fp );
   fwrite( data->pos->data , sizeof(double) , len , fp );

   return TRUE;
}

gboolean load_measuring_station_data( gpointer ptr , FILE *fp )
{
   Met_station_t *data = (Met_station_t*)ptr;
   guint len;
   double *pos_data;

   fread( &(data->initialized)      , sizeof(gboolean) , 1 , fp );
   fread( &(data->from_river_mouth) , sizeof(gboolean) , 1 , fp );

   fread( &len , sizeof(guint) , 1 , fp );
   fread( data->filename , sizeof(char) , len , fp );

   fread( &len , sizeof(guint) , 1 , fp );
   fread( data->parameter_str , sizeof(char) , len , fp );

   fread( &len , sizeof(guint) , 1 , fp );
   pos_data = eh_new( double , len );
   fread( pos_data, sizeof(double) , len , fp );

   data->pos = g_array_new( FALSE , FALSE , sizeof(double) );
   g_array_append_vals( data->pos , pos_data , len );

   fread( &len , sizeof(guint) , 1 , fp );
   data->filename = eh_new( char , len );
   fread( data->filename , sizeof(char) , len , fp );

   eh_free( pos_data );

   return FALSE;
}
/*
int write_measurement_header( FILE *fp , 
                              Met_station_t *measuring_station_const )
{
   int i;
   GArray *pos=measuring_station_const->pos;

   fprintf( fp , "Measuring station output file.\n" );
   fprintf( fp , "Byte order : %d\n" , G_BYTE_ORDER );
   fprintf( fp , "Measuring parameter : %s\n" ,
                 measuring_station_const->parameter_str );
   fprintf( fp , "Measuring station location(s) relative to : " );

   if ( measuring_station_const->from_river_mouth )
      fprintf( fp , "river mouth\n" );
   else
      fprintf( fp , "origin\n" );
   fprintf( fp , "Measuring station location(s) : " );
   if ( pos->len == 0 )
      fprintf( fp , "all locations\n" );
   else
   {
      for (i=0;i<pos->len-1;i++)
         fprintf( fp , "%f," , g_array_index(pos,double,i) );
      fprintf( fp , "%f\n" , g_array_index(pos,double,pos->len-1) );
   }
   return 0;
}
*/

