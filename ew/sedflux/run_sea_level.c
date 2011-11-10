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

#define SED_SEA_LEVEL_PROC_NAME "sea level"
#define EH_LOG_DOMAIN SED_SEA_LEVEL_PROC_NAME

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <utils/utils.h>
#include "my_processes.h"

double**  read_sea_level_curve( char* , gint* );
double    get_sea_level       ( double** , gint , double );

gboolean init_sea_level_data( Sed_process proc , Sed_cube prof , GError** error );

Sed_process_info
run_sea_level( Sed_process proc , Sed_cube prof )
{
   Sea_level_t*     data = (Sea_level_t*)sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   double new_sea_level;
   double year;

   if ( sed_process_run_count(proc)==0 )
      init_sea_level_data( proc , prof , NULL );

   year = sed_cube_age_in_years( prof )-data->start_year;

   new_sea_level = get_sea_level( data->sea_level , data->len , year );

   if ( eh_isnan( new_sea_level ) )
   {
      eh_warning( "The current time is out of range of the sea level curve." );
      eh_warning( "Sea level will be held constant." );
   }
   else
      sed_cube_set_sea_level(prof,new_sea_level);

   if ( eh_isnan(sed_cube_sea_level(prof)) )
      return info;

   sed_cube_find_all_river_mouths( prof );

   eh_message( "time      : %f" , sed_cube_age_in_years(prof) );
   eh_message( "sea level : %f" , sed_cube_sea_level(prof)    );

   return info;
}

#include <sys/stat.h>

#define SEA_LEVEL_KEY_FILENAME "sea level file"

static const gchar* sea_level_req_labels[] =
{
   SEA_LEVEL_KEY_FILENAME ,
   NULL
};

gboolean
init_sea_level( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Sea_level_t* data    = sed_process_new_user_data( p , Sea_level_t );
   GError*      tmp_err = NULL;
   gboolean     is_ok   = TRUE;
   gint         len;

   eh_message ("initializing sea level");
   data->start_year = 0.;

   eh_symbol_table_require_labels( tab , sea_level_req_labels , &tmp_err );

   if ( !tmp_err )
   {
     gchar* prefix = sed_process_prefix (p);
     gchar* file = NULL;

     if (!prefix)
       prefix = g_strdup (".");

     file = eh_symbol_table_value (tab, SEA_LEVEL_KEY_FILENAME);

     /* If file is an absolute path name, don't append the prefix */
     if (file[0]=='/')
       data->filename = g_strdup (file);
     else
       data->filename = g_build_filename (prefix, file, NULL);
     data->sea_level = sed_scan_sea_level_curve (data->filename, &len,
                                                 &tmp_err);
     data->len = len;

     g_free (file);
     g_free (prefix);
   }

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
      eh_message ("There was an error: %s", (*error)->message);
      eh_exit (-1);
   }
   eh_message ("done");

   return is_ok;
}

gboolean
init_sea_level_data( Sed_process proc , Sed_cube prof , GError** error )
{
   gboolean     is_ok = TRUE;
   Sea_level_t* data  = (Sea_level_t*)sed_process_user_data( proc );

   if ( data )
      data->start_year = sed_cube_age_in_years(prof);

   return is_ok;
}

gboolean
destroy_sea_level( Sed_process p )
{
   if ( p )
   {
      Sea_level_t* data = (Sea_level_t*)sed_process_user_data( p );
      
      if ( data )
      {
         eh_free_2( data->sea_level );

         eh_free  ( data->filename   );
         eh_free  ( data             );
      }
   }

   return TRUE;
}

gboolean dump_sea_level_data( gpointer ptr , FILE *fp )
{
   Sea_level_t *data = (Sea_level_t*)ptr;
   guint len;

   len = strlen(data->filename);
   fwrite( &len , sizeof(guint) , 1 , fp );
   fwrite( data->filename , sizeof(char) , 1 , fp );

   fwrite( &(data->start_year) , sizeof(double) , 1 , fp );

   fwrite( &(data->len) , sizeof(gint) , 1 , fp );
   fwrite( data->sea_level[0] , sizeof(double) , data->len , fp );
   fwrite( data->sea_level[1] , sizeof(double) , data->len , fp );

   return TRUE;
}

gboolean load_sea_level_data( gpointer ptr , FILE *fp )
{
   Sea_level_t *data = (Sea_level_t*)ptr;
   guint len;

   fread( &len , sizeof(guint) , 1 , fp );
   fread( data->filename , sizeof(char) , len , fp );
   fread( &(data->start_year) , sizeof(double) , 1 , fp );

   fread( &len , sizeof(gint) , 1 , fp );

   data->len       = len;
   data->sea_level = eh_new_2( double , 2 , len );

   fread( data->sea_level[0] , sizeof(double) , len , fp );
   fread( data->sea_level[1] , sizeof(double) , len , fp );

   return TRUE;
}

double get_sea_level(double** sea_level, gint len , double year)
{
   double new_sea_level;

   eh_require( sea_level );

   {
      double* t  = sea_level[0];
      double* z  = sea_level[1];

      interpolate(t,z,len,&year,&new_sea_level,1);
   }

   return new_sea_level;
}

