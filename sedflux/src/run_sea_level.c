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

Eh_data_record read_sea_level_curve(char *filename);
double get_sea_level(Eh_data_record,double);

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
         eh_data_record_destroy( data->sea_level );
         data->initialized = FALSE;
      }
      return SED_EMPTY_INFO;
   }

   if ( !data->initialized )
   {
      data->sea_level = read_sea_level_curve(data->filename);
      data->start_year = sed_cube_age_in_years(prof);
      data->initialized = TRUE;
   }

   year = sed_cube_age_in_years( prof )-data->start_year;

   new_sea_level = get_sea_level( data->sea_level , year );

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
      eh_exit(-1);

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

   len = eh_data_record_size( data->sea_level , 1 );
   t   = eh_data_record_row ( data->sea_level , 0 );
   z   = eh_data_record_row ( data->sea_level , 1 );
   fwrite( &len , sizeof(guint) , 1 , fp );
   fwrite( t , sizeof(double) , len , fp );
   fwrite( z , sizeof(double) , len , fp );

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

   fread( &len , sizeof(guint) , 1 , fp );

   t = eh_new( double , len );
   z = eh_new( double , len );

   fread( t , sizeof(double) , len , fp );
   fread( z , sizeof(double) , len , fp );

   data->sea_level = eh_data_record_new();

   eh_data_record_add_row( data->sea_level , t );
   eh_data_record_add_row( data->sea_level , z );

   eh_free( t );
   eh_free( z );

   return TRUE;
}

double get_sea_level(Eh_data_record sea_level, double year)
{
   double new_sea_level;

   eh_require( sea_level );

   {
      double* t  = eh_data_record_row ( sea_level , 0 );
      double* z  = eh_data_record_row ( sea_level , 1 );
      gssize len = eh_data_record_size( sea_level , 1 );

      interpolate(t,z,len,&year,&new_sea_level,1);
   }

   return new_sea_level;
}

Eh_data_record read_sea_level_curve(char *filename)
{
   Eh_data_record sea_level_curve;

   eh_require( filename );

   {
      gssize i;
      Eh_data_record* all_records;

      all_records = eh_data_record_scan_file( filename , "," , EH_FAST_DIM_COL , FALSE );

      if ( all_records )
         sea_level_curve = all_records[0];
      else
         eh_error( "Problem reading sea-level curve from %s" , filename );

      for ( i=1 ; all_records[i] ; i++ )
         eh_data_record_destroy( all_records[i] );
      eh_free( all_records );
   }

   return sea_level_curve;
}

