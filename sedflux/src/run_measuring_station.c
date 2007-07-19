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
#include <fcntl.h>
#include <string.h>
#include <limits.h>

#ifndef NAME_MAX
# define NAME_MAX 255
#endif

#include "utils.h"
#include "sed_sedflux.h"
#include "my_processes.h"

/*
int write_measurement_header( FILE *fp , 
                              Met_station_t *measuring_station_const );
*/

gboolean init_met_station_data( Sed_process proc , Sed_cube prof , GError** error );

Sed_process_info
run_met_station( Sed_process proc , Sed_cube prof )
{
   Met_station_t*   data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;

   if ( sed_process_run_count(proc)==0 )
      init_met_station_data( proc , prof , NULL );

   sed_tripod_write( data->met_fp , prof );

   return info;
}

#define MET_KEY_PARAMETER  "parameter to measure"
#define MET_KEY_WHENCE     "position wrt river mouth"
#define MET_KEY_POSITION   "position of station"
#define MET_KEY_FILENAME   "filename"

static gchar* measuring_station_req_labels[] =
{
   MET_KEY_PARAMETER ,
   MET_KEY_WHENCE    ,
   MET_KEY_POSITION  ,
   MET_KEY_FILENAME  ,
   NULL
};

gboolean
init_met_station( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Met_station_t* data    = sed_process_new_user_data( p , Met_station_t );
   GError*        tmp_err = NULL;
   gboolean       is_ok   = TRUE;
   gchar*         pos_s   = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   data->met_fp           = NULL;

   if ( eh_symbol_table_require_labels( tab , measuring_station_req_labels , &tmp_err ) )
   {
      data->parameter_str    = eh_symbol_table_value     ( tab , MET_KEY_PARAMETER );
      data->from_river_mouth = eh_symbol_table_bool_value( tab , MET_KEY_WHENCE    );
      data->filename         = eh_symbol_table_value     ( tab , MET_KEY_FILENAME  );
      data->parameter        = sed_measurement_new( data->parameter_str );

      pos_s                  = eh_symbol_table_lookup( tab , MET_KEY_POSITION );

      if ( g_ascii_strcasecmp( pos_s , "all" ) == 0 )
         data->pos = g_array_new( FALSE , FALSE , sizeof(double) );
      else
      {
         gint    i;
         double  d_val;
         gchar** position = g_strsplit ( pos_s , "," , -1 );

         data->pos = g_array_new( FALSE , FALSE , sizeof(double) );
         for ( i=0 ; !tmp_err && position[i] ; i++ )
         {
            d_val = eh_str_to_dbl( position[i] , &tmp_err );

            if ( !tmp_err ) g_array_append_val( data->pos , d_val );
         }
         g_strfreev(position);
      }

      eh_touch_file( data->filename , O_WRONLY|O_CREAT , &tmp_err );
   }

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
init_met_station_data( Sed_process proc , Sed_cube prof , GError** error )
{
   Met_station_t* data = sed_process_user_data( proc );

   if ( data )
   {
      data->met_fp = sed_tripod_new( data->filename  , data->parameter , NULL );

      if ( data->pos->len <= 0 )
         sed_tripod_set_len( data->met_fp , sed_cube_size(prof) );
      else
         sed_tripod_set_len( data->met_fp , data->pos->len      );

      sed_tripod_set_n_x( data->met_fp , sed_cube_n_x(prof) );
      sed_tripod_set_n_y( data->met_fp , sed_cube_n_y(prof) );
   }

   return TRUE;
}

gboolean
destroy_met_station( Sed_process p )
{
   if ( p )
   {
      Met_station_t* data = sed_process_user_data( p );

      if ( data )
      {
         sed_tripod_destroy( data->met_fp );

         if ( data->pos ) g_array_free( data->pos , FALSE );

         sed_measurement_destroy( data->parameter );

         eh_free( data->parameter_str );
         eh_free( data->filename      );
         eh_free( data                );
      }
   }

   return TRUE;
}

gboolean dump_measuring_station_data( gpointer ptr , FILE *fp )
{
   Met_station_t *data = (Met_station_t*)ptr;
   gint64 len;

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

