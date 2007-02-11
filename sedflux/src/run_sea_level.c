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

#include "utils.h"
#include "sealevel.h"

double**  read_sea_level_curve( char* , gint* );
double    get_sea_level       ( double** , gint , double );

Sed_process_info run_sea_level(gpointer ptr,Sed_cube prof)
{
   Sea_level_t *data=(Sea_level_t*)ptr;
   double new_sea_level;
   double year;
   Sed_process_info info = SED_EMPTY_INFO;

   if ( prof == NULL )
   {
      if ( data->initialized )
      {
         eh_free_2( data->sea_level );
         data->initialized = FALSE;
      }
      return SED_EMPTY_INFO;
   }

   if ( !data->initialized )
   {
      gint len;
      GError* err = NULL;

      data->sea_level   = sed_scan_sea_level_curve( data->filename , &len , &err );

      if ( err )
      {
         fprintf( stderr , "Error reading sea level file: %s\n" , err->message );
         eh_exit( EXIT_FAILURE );
      }

      data->len         = len;
      data->start_year  = sed_cube_age_in_years(prof);
      data->initialized = TRUE;
   }

   year = sed_cube_age_in_years( prof )-data->start_year;

   new_sea_level = get_sea_level( data->sea_level , data->len , year );

   if ( eh_isnan( new_sea_level ) )
   {
      eh_warning( "The current time is out of range of the sea level curve." );
      eh_warning( "Sea level will be held constant constant." );
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

#define S_KEY_SEA_LEVEL_FILE "sea level file"

gboolean init_sea_level(Eh_symbol_table symbol_table,gpointer ptr)
{
   Sea_level_t *data=(Sea_level_t*)ptr;
   if ( symbol_table == NULL )
   {
      eh_free( data->filename );
      data->initialized = FALSE;
      return TRUE;
   }

   data->filename = eh_symbol_table_value( symbol_table, S_KEY_SEA_LEVEL_FILE );

   if ( !eh_try_open(data->filename) )
      eh_exit( EXIT_FAILURE );

   return TRUE;
}

gboolean dump_sea_level_data( gpointer ptr , FILE *fp )
{
   Sea_level_t *data = (Sea_level_t*)ptr;
   guint len;
   double *t, *z;

   fwrite( &(data->initialized) , sizeof(gboolean) , 1 , fp );

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
   double *t, *z;

   fread( &(data->initialized) , sizeof(gboolean) , 1 , fp );

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

