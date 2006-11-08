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

#define SED_CPR_PROC_NAME "cpr"
#define EH_LOG_DOMAIN SED_CPR_PROC_NAME

#include "utils.h"
#include "sed_sedflux.h"
#include "cpr.h"
#include <string.h>

gboolean dump_cpr_data( gpointer ptr , FILE *fp );
gpointer load_cpr_data( FILE *fp );
void eh_dump_file_list( Eh_file_list *fl , FILE *fp );
Eh_file_list *eh_load_file_list( FILE *fp );

Sed_process_info run_cpr(gpointer ptr,Sed_cube prof)
{
   Cpr_t *data=(Cpr_t*)ptr;
   char *file_name;
   char *base_name;
   FILE *fp;

   if ( prof == NULL )
   {
      if ( data->initialized )
      {
         data->initialized = FALSE;
      }
      return SED_EMPTY_INFO;
   }

   if ( !data->initialized )
   {
      gchar* cube_name = sed_cube_name( prof );
      base_name = g_strconcat( data->output_dir  ,
                               G_DIR_SEPARATOR_S ,
                               cube_name         ,
                               "#"               ,
                               ".cpr" , NULL );
      data->file_list = eh_create_file_list( base_name );
      eh_free( base_name );
      eh_free( cube_name );

      data->initialized = TRUE;
   }

   file_name = eh_get_next_file( data->file_list );

   fp = fopen( file_name , "wb" );

   sed_cube_write( fp , prof );

   fclose(fp);

   return SED_EMPTY_INFO;
}

#define S_KEY_DIR       "output directory"

gboolean init_cpr( Eh_symbol_table symbol_table , gpointer ptr )
{
   Cpr_t *data=(Cpr_t*)ptr;
   
   if ( symbol_table == NULL )
   {
      eh_free( data->output_dir );
      data->output_dir = NULL;
      data->initialized = FALSE;
      return TRUE;
   }

   data->output_dir = eh_symbol_table_value( symbol_table , S_KEY_DIR );

   if ( !try_dir( data->output_dir ) )
      eh_exit(-1);

   return TRUE;
}

gboolean dump_cpr_data( gpointer ptr , FILE *fp )
{
   Cpr_t *data=(Cpr_t*)ptr;
   gint len;

   eh_require( ptr!=NULL );
   eh_require( fp!=NULL  );

   len = strlen( data->output_dir )+1;

   eh_dump_file_list( data->file_list , fp );
   fwrite( &len                 , sizeof(gint)      , 1   , fp );
   fwrite( data->output_dir     , sizeof(char)      , len , fp );
   fwrite( &(data->initialized) , sizeof(gboolean)  , 1   , fp );

   return TRUE;
}

gpointer load_cpr_data( FILE *fp )
{
   Cpr_t *data;
   gint len;

   eh_require( fp!=NULL );

   data = eh_new( Cpr_t , 1 );

   data->file_list = eh_load_file_list( fp );
   fread( &len                 , sizeof(gint) , 1   , fp );
   fread( data->output_dir     , sizeof(char) , len , fp );
   fread( &(data->initialized) , sizeof(char) , len , fp );

   return (gpointer)data;
}

void eh_dump_file_list( Eh_file_list *fl , FILE *fp )
{
   int len;

   len = strlen( fl->prefix );
   fwrite( &len         , sizeof(int)  , 1   , fp );
   fwrite( fl->prefix   , sizeof(char) , len , fp );

   len = strlen( fl->suffix );
   fwrite( &len         , sizeof(int)  , 1   , fp );
   fwrite( fl->suffix   , sizeof(char) , len , fp );

   len = strlen( fl->format );
   fwrite( &len         , sizeof(int)  , 1   , fp );
   fwrite( fl->suffix   , sizeof(char) , len , fp );

   fwrite( &(fl->count) , sizeof(int)  , 1   , fp );

}

Eh_file_list *eh_load_file_list( FILE *fp )
{
   int len;
   Eh_file_list *fl = eh_new( Eh_file_list , 1 );

   fread( &len , sizeof(int) , 1 , fp );
   fl->prefix = eh_new( char , len );
   fread( fl->prefix   , sizeof(char) , len , fp );

   fread( &len , sizeof(int) , 1 , fp );
   fl->suffix = eh_new( char , len );
   fread( fl->suffix   , sizeof(char) , len , fp );

   fread( &len , sizeof(int) , 1 , fp );
   fl->format = eh_new( char , len );
   fread( fl->format   , sizeof(char) , len , fp );

   fread( &(fl->count) , sizeof(int)  , 1   , fp );

   return fl;
}

