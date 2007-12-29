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

#define SED_DATA_DUMP_PROC_NAME "data dump"
#define EH_LOG_DOMAIN SED_DATA_DUMP_PROC_NAME

#include <string.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "my_processes.h"

Sed_process_info
run_data_dump( Sed_process proc , Sed_cube prof )
{
   Data_dump_t*     data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   int i;
   char str[S_NAMEMAX], *filename;
   gchar* cube_name;
   Sed_property_file fp;
   Sed_property_file_attr attr;
   Sed_property property;

   data->count++;
   sprintf(str,"%04d",data->count);

   cube_name = sed_cube_name( prof );
   for (i=0; data->property && i<data->property->len ;i++)
   {
      property = sed_property_dup( g_array_index( data->property , Sed_property , i ) );

      filename = g_strconcat( data->output_dir  ,
                              G_DIR_SEPARATOR_S ,
                              cube_name         ,
                              str ,
                              "." ,
                              sed_property_extension(property) , NULL );

      attr = sed_property_file_attr_new( );

eh_warning( "property file attributes are not being used." );
/*
      sed_set_sed_file_attr_y_res( attr , data->vertical_resolution );
      sed_set_sed_file_attr_x_res( attr , data->horizontal_resolution );
      sed_set_sed_file_attr_y_lim( attr , data->y_lim_min , data->y_lim_max );
      sed_set_sed_file_attr_x_lim( attr , data->x_lim_min , data->x_lim_max );
*/

      fp = sed_property_file_new( filename , property , NULL );

      sed_property_file_write( fp , prof );
      sed_property_file_attr_destroy( attr );
      sed_property_file_destroy( fp );

      eh_message( "time                           : %f" ,
                  sed_cube_age_in_years(prof) );
      eh_message( "filename                       : %s" , filename        );
      eh_message( "vertical resolution (0=full)   : %f" ,
                  data->vertical_resolution );
      eh_message( "horizontal resolution (0=full) : %f" ,
                  data->horizontal_resolution );

      eh_free(filename);
   }
   eh_free( cube_name );

   return info;
}

#define DATA_DUMP_KEY_DIR       "output directory"
#define DATA_DUMP_KEY_VRES      "vertical resolution"
#define DATA_DUMP_KEY_HRES      "horizontal resolution"
#define DATA_DUMP_KEY_Y_LIM     "vertical limits"
#define DATA_DUMP_KEY_X_LIM     "horizontal limits"
#define DATA_DUMP_KEY_PROPERTY  "property"

static gchar* data_dump_req_labels[] =
{
   DATA_DUMP_KEY_DIR      ,
   DATA_DUMP_KEY_VRES     ,
   DATA_DUMP_KEY_HRES     ,
   DATA_DUMP_KEY_Y_LIM    ,
   DATA_DUMP_KEY_X_LIM    ,
   DATA_DUMP_KEY_PROPERTY ,
   NULL
};

gboolean
init_data_dump( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Data_dump_t* data    = sed_process_new_user_data( p , Data_dump_t );
   GError*      tmp_err = NULL;
   gboolean     is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );
   
   if ( eh_symbol_table_require_labels( tab , data_dump_req_labels , &tmp_err ) )
   {
      data->property   = g_array_new( FALSE , FALSE , sizeof(Sed_property) );
      data->output_dir = eh_symbol_table_value( tab , DATA_DUMP_KEY_DIR );

      // ---
      // read the vertical and horizontal resolutions.  if the key word, 'full'
      // is given for either resolution, use the full resolution for that 
      // direction.
      // ---
      if ( strcasecmp( eh_symbol_table_lookup( tab , DATA_DUMP_KEY_VRES ) , "full" ) == 0 )
         data->vertical_resolution = -1;
      else
         data->vertical_resolution = eh_symbol_table_dbl_value( tab , DATA_DUMP_KEY_VRES );

      if ( strcasecmp( eh_symbol_table_lookup( tab , DATA_DUMP_KEY_HRES ) , "full" ) == 0 )
         data->horizontal_resolution = -1;
      else
         data->horizontal_resolution = eh_symbol_table_dbl_value( tab , DATA_DUMP_KEY_HRES );

      // ---
      // read the x and y limits of the output file.  if the key word, 'tight'
      // is given for either limits, use limits that will encompass all of the
      // data in that direction.
      // ---
      if ( strcasecmp( eh_symbol_table_lookup( tab , DATA_DUMP_KEY_Y_LIM ) , "tight") == 0 ) {
         data->y_lim_min = -G_MAXDOUBLE;
         data->y_lim_max =  G_MAXDOUBLE;
      }
      else
      {
         gchar** y_lim_str = g_strsplit( eh_symbol_table_lookup( tab , DATA_DUMP_KEY_Y_LIM ) , "," , 2 );
         data->y_lim_min = strtod( y_lim_str[0] , NULL );
         data->y_lim_max = strtod( y_lim_str[1] , NULL );
      }

      if ( strcasecmp( eh_symbol_table_lookup( tab , DATA_DUMP_KEY_X_LIM ) , "tight" ) == 0 )
      {
         data->x_lim_min = -G_MAXDOUBLE;
         data->x_lim_max =  G_MAXDOUBLE;
      }
      else
      {
         gchar** x_lim_str = g_strsplit( eh_symbol_table_lookup( tab , DATA_DUMP_KEY_X_LIM ) , "," , 2 );
         data->x_lim_min = strtod( x_lim_str[0] , NULL );
         data->x_lim_max = strtod( x_lim_str[1] , NULL );
      }

      // ---
      // read the property labels.  they are comma delimited so we cycle through
      // the list of them.
      // ---
      {
         gint         i;
         gchar**      property = g_strsplit( eh_symbol_table_lookup( tab , DATA_DUMP_KEY_PROPERTY ) , "," , -1 );
         Sed_property property_val;

         for ( i=0 ; property[i] ; i++ )
         {
            property_val = sed_property_new( property[i] );
            g_array_append_val( data->property , property_val );
         }

         g_strfreev(property);
      }
   
//   if ( !try_dir(data->output_dir) )
      if ( !tmp_err ) eh_open_dir( data->output_dir , &tmp_err );
   }

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
destroy_data_dump( Sed_process p )
{
   if ( p )
   {
      Data_dump_t* data = sed_process_user_data( p );

      if ( data )
      {
         if ( data->property ) g_array_free( data->property , TRUE );
         eh_free( data->output_dir );
         eh_free( data             );
      }
   }

   return TRUE;
}

gboolean dump_data_dump_data( gpointer ptr , FILE *fp )
{
   guint len;
   Data_dump_t *data = (Data_dump_t*)ptr;

   fwrite( &(data->vertical_resolution) , sizeof(double) , 1 , fp );
   fwrite( &(data->horizontal_resolution) , sizeof(double) , 1 , fp );
   fwrite( &(data->y_lim_min) , sizeof(double) , 1 , fp );
   fwrite( &(data->y_lim_max) , sizeof(double) , 1 , fp );
   fwrite( &(data->x_lim_min) , sizeof(double) , 1 , fp );
   fwrite( &(data->x_lim_max) , sizeof(double) , 1 , fp );

   len = strlen( data->output_dir );
   fwrite( &len , sizeof(guint) , 1 , fp );
   fwrite( data->output_dir , sizeof(char) , len , fp );

   len = data->property->len;
   fwrite( &len  , sizeof(guint) , 1 , fp );
   fwrite( &(data->property->data) , sizeof(guint) , len , fp );

   fwrite( &(data->count) , sizeof(int) , 1 , fp );

   return TRUE;
}

gboolean load_data_dump_data( gpointer ptr , FILE *fp )
{
   Data_dump_t *data = (Data_dump_t*)ptr;
   Sed_property *property;
   guint len;

   fread( &(data->vertical_resolution)   , sizeof(double) , 1 , fp );
   fread( &(data->horizontal_resolution) , sizeof(double) , 1 , fp );
   fread( &(data->y_lim_min)             , sizeof(double) , 1 , fp );
   fread( &(data->y_lim_max)             , sizeof(double) , 1 , fp );
   fread( &(data->x_lim_min)             , sizeof(double) , 1 , fp );
   fread( &(data->x_lim_max)             , sizeof(double) , 1 , fp );

   fread( &len                           , sizeof(guint)  , 1 , fp );
   data->output_dir = eh_new( char , len );
   fread( data->output_dir , sizeof(char) , len , fp );

   data->property = g_array_new( FALSE , FALSE , sizeof(Sed_property) );
   fread( &len , sizeof(guint) , 1 , fp );
   property = eh_new( Sed_property , len );
   fread( property , sizeof(Sed_property) , len , fp );
   g_array_append_vals( data->property , property , len );

   fread( &(data->count) , sizeof(int) , 1 , fp );

   return TRUE;
}

