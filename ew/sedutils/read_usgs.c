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
#include <glib.h>
#include <utils/utils.h>

static const char *help_msg[] = {
" read_usgs - read a usgs daily values file.           ",
"                                                      ",
" options:                                             ",
"  nrecs : the number of records to read  [10]         ",
"  start : start reading the data records at the       ",
"        : start-th record [365]                       ",
"  del   : the character to delimit the output with [,]",
"  rm    : if yes the output will be written row by    ",
"        : row [no]                                    ",
"  head  : if yes, a header will be written before the ",
"        : data [no]                                   ",
"  bo    : specify the byte order of the data [be]     ",
"        : possible values are:                        ",
"        :  be - big endian                            ",
"        :  le - little endian                         ",
"  units : specify the units of the source data [none] ",
"        : possible values are:                        ",
"        :  o cfs  - cubic feet per second.  the       ",
"        :    equivalent si units are m^3/s.           ",
"        :  o ft   - feet.  the equivalent si units are",
"        :    meters.                                  ",
"        :  o tn   - tonnes per day.  the equivalent   ",
"        :    si units are kg/s.                       ",
"        :  none - no units                            ",
"        : these units will be converted to            ",
"        : corresponding the si units.                 ",
"  scale : specify your own scale factor.  this is in  ",
"        : addition to any scaling that may be done in ",
"        : converting to si units. [1.]                ",
"  fmt   : specify the format of the daily values.     ",
"        :  usgs - usgs daily value format. [default]  ",
"        :  cdn  - canadian daily value format.        ",
"  v     : be verbose [no]                             ",
"                                                      ",
NULL };

typedef struct
{
   char blank1[2];
   char state[2];
   char agency[5];
   char id[15];
   gfloat sample_xsecloc;
   gfloat sample_depth;
   gint32 parameter;
   gint16 year;
   gint16 statistic;
   gfloat novalue_mark;
   gfloat flows[12][31];
   char blank2[3];
   char agency_office[2];
   char county[3];
   char staname[48];
   gfloat drain_area;
   gfloat cont_area;
   gfloat well_depth;
   char datum[4];
   gint32 hydro_unit;
   gint16 retrieval;
   gint16 start_month;
   char sitecode[2];
   char latitude[6];
   char longitude[7];
   char seqnum[2];
   char geo_unit[8];
   char blank3[2];
   char aquifer[1];
   char blank4[9];
   char blank5[7];
}
daily_val_st;

typedef struct
{
   char station_number[7];
   gint8 parameter;
   gint16 year;
   gfloat daily_values[12][31];
   char flags[12][31];
}
daily_cdn_val_st;

#define METERS_PER_FOOT    ( 0.3048 )
#define KG_PER_TONNE_LONG  ( 1.016260162601626e+03 )
#define KG_PER_TONNE_SHORT ( 9.090909090909090e+02 )
#define SECONDS_PER_DAY    ( 86400. )
#define NO_CONVERSION      ( 1. )

int read_usgs_daily_value_record( daily_val_st *rec , FILE *fp , int byte_order );
int read_cdn_daily_value_record( daily_val_st *rec , FILE *fp , int byte_order );

int main(int argc, char *argv[])
{
   char *byte_order_vals[] = { "be" , "le" , NULL };
   char *unit_vals[] = { "none" , "cfs" , "ft" , "tn" , NULL };
   char *fmt_vals[] = { "usgs" , "cdn" , NULL };
   char *base_name;
   int byte_order, format, units;
   int n_recs, start;
   double scale;
   char *delimeter, *in_file;
   gboolean row_major, header;
   gboolean verbose;
   FILE *fp_in;
   int i, j, k;
   double convert_factor;
   double val;
   double time;
   daily_val_st *new_rec;
   GList *records=NULL; // a doubly-linked list of daily_val_st-s
   Eh_args *args;
   Eh_data_record data;
   GArray *data_array = g_array_new( FALSE , FALSE , sizeof(double) );
   GArray *time_array = g_array_new( FALSE , FALSE , sizeof(double) );

   args = eh_opts_init(argc,argv);
   if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
      eh_exit(-1);

   n_recs     = eh_get_opt_int ( args , "nrecs" , G_MAXINT );
   start      = eh_get_opt_int ( args , "start" , 0        );
   scale      = eh_get_opt_dbl ( args , "scale" , 1        );
   delimeter  = eh_get_opt_str ( args , "del"   , ","      );
   in_file    = eh_get_opt_str ( args , "in"    , "-"      );
   row_major  = eh_get_opt_bool( args , "rm"    , FALSE    );
   header     = eh_get_opt_bool( args , "head"  , FALSE    );
   verbose    = eh_get_opt_bool( args , "v"     , FALSE    );
   byte_order = eh_get_opt_key ( args , "bo"    , 0 , byte_order_vals );
   format     = eh_get_opt_key ( args , "fmt"   , 0 , fmt_vals );
   units      = eh_get_opt_key ( args , "units" , 0 , unit_vals );

   byte_order = (byte_order==0)?G_BIG_ENDIAN:G_LITTLE_ENDIAN;
   switch ( units )
   {
      case 1:
         convert_factor = METERS_PER_FOOT*METERS_PER_FOOT*METERS_PER_FOOT;
         break;
      case 2:
         convert_factor = METERS_PER_FOOT;
         break;
      case 3:
         convert_factor = KG_PER_TONNE_SHORT/SECONDS_PER_DAY;
         break;
      default:
         convert_factor = NO_CONVERSION;
   }
   convert_factor *= scale;
   if ( strcmp( in_file , "-" )==0 )
      fp_in = stdin;
   else
      fp_in = fopen( in_file , "r" );

   if ( !fp_in )
      perror( in_file ), eh_exit(-1);

   if ( verbose )
   {
      fprintf(stderr,"host byte order   : %d\n",G_BYTE_ORDER);
      fprintf(stderr,"source byte order : %d\n",byte_order);
      fprintf(stderr,"source units      : %s\n",unit_vals[units]);
   }

// read in the data.
   data = eh_data_record_new();

   if ( format==0 )
      fseek(fp_in,start*sizeof(daily_val_st),SEEK_SET);
   else
      fseek(fp_in,start*sizeof(daily_cdn_val_st),SEEK_SET);

   for ( i=0 ; i<n_recs ; i++ )
   {
//      data_array = g_array_new( FALSE , FALSE , sizeof(double) );
      new_rec = g_new0( daily_val_st , 1 );

// read one record (one year of daily values)
      if ( format==0 )
         read_usgs_daily_value_record( new_rec , fp_in , byte_order );
      else
         read_cdn_daily_value_record( new_rec , fp_in , byte_order );

// break out of the loop if EOF was encountered.
      if ( feof(fp_in) )
      {
         fprintf(stderr,"warning : reached and of file after only %d records.\n",i);
         g_free(new_rec);
         break;
      }

// add the daily values to the data file.
      for ( j=0 ; j<12 ; j++ )
         for ( k=0 ; k<31 ; k++ )
         {
            val = new_rec->flows[j][k];
            if ( val > new_rec->novalue_mark+1e-5 )
               val *= convert_factor;
            time = j*31 + k + (new_rec->year-1900)*(12*31);
            g_array_append_val( time_array , time );
            g_array_append_val( data_array , val  );
         }
      records = g_list_append( records , new_rec );
//      eh_add_data_record_row( data , data_array );
   }

   eh_data_record_add_row( data , (double*)(time_array->data) );
   eh_data_record_add_row( data , (double*)(data_array->data) );
   

// add the data to a Eh_data_record.
   new_rec = g_list_nth_data( records , 0 );
   eh_data_record_add_label( data , "station"         , g_strndup(new_rec->staname,48) );
   eh_data_record_add_label( data , "site code"       , g_strndup(new_rec->sitecode,2) );
   eh_data_record_add_label( data , "latitude"        , g_strndup(new_rec->latitude,6) );
   eh_data_record_add_label( data , "longitude"       , g_strndup(new_rec->longitude,7) );
   eh_data_record_add_label( data , "parameter"       , g_strdup_printf("%d",new_rec->parameter) );
   eh_data_record_add_label( data , "statistic"       , g_strdup_printf("%d",new_rec->statistic) );
   eh_data_record_add_label( data , "no value"        , g_strdup_printf("%f",new_rec->novalue_mark) );
   eh_data_record_add_label( data , "start month"     , g_strdup_printf("%d",new_rec->start_month) );
   eh_data_record_add_label( data , "start year"      , g_strdup_printf("%d",new_rec->year) );
   eh_data_record_add_label( data , "number of years" , g_strdup_printf("%d",g_list_length(records)) );

   base_name = g_path_get_basename( argv[0] );

// write the data.
   eh_data_record_print( data      ,
                         base_name ,
                         delimeter ,
                         row_major ,
                         header    ,
                         stdout );

   g_free( base_name );

   return 0;
}


gfloat gfloat_from_le( gfloat a );

gfloat gfloat_from_le( gfloat a )
{
   gint32 *dummy = (gint32*)&a;
   *dummy = GINT32_FROM_LE( *dummy );
   return a;
}

int read_usgs_daily_value_record( daily_val_st *rec , FILE *fp , int byte_order )
{
   int i, j;
   fread(rec,sizeof(daily_val_st),1,fp);

   if ( byte_order != G_BYTE_ORDER )
   {
      rec->sample_xsecloc = gfloat_from_le(rec->sample_xsecloc);
      rec->sample_depth   = gfloat_from_le(rec->sample_depth);
      rec->parameter      = GINT32_FROM_LE(rec->parameter);
      rec->year           = GINT16_FROM_LE(rec->year);
      rec->statistic      = GINT16_FROM_LE(rec->statistic);
      rec->novalue_mark  =  gfloat_from_le(rec->novalue_mark);
      for ( i=0 ; i<12 ; i++ )
         for ( j=0 ; j<31 ; j++ )
            rec->flows[i][j] = gfloat_from_le(rec->flows[i][j]);
      rec->drain_area     = gfloat_from_le(rec->drain_area);
      rec->cont_area      = gfloat_from_le(rec->cont_area);
      rec->well_depth     = gfloat_from_le(rec->well_depth);
      rec->hydro_unit     = GINT32_FROM_LE(rec->hydro_unit);
      rec->retrieval      = GINT16_FROM_LE(rec->retrieval);
      rec->start_month    = GINT16_FROM_LE(rec->start_month);

   }

   return 0;
}

int read_cdn_daily_value_record( daily_val_st *rec , FILE *fp , int byte_order )
{
   int i, j;
   char *station_number = g_new0( char , 7 );
   gint8 parameter;
   gint16 year;
   gfloat **values;
   char **flags;

   values = g_new0( gfloat* , 12 );
   values[0] = g_new0( gfloat , 12*31 );
   flags = g_new0( char* , 12 );
   flags[0] = g_new0( char , 12*31 );

   for ( i=1 ; i<12 ; i++ )
   {
      flags[i]  = flags[i-1]+31;
      values[i] = values[i-1]+31;
   }

fread( station_number , sizeof(char)   , 7     , fp );
fread( &parameter     , sizeof(gint8)  , 1     , fp );
fread( &year          , sizeof(gint16) , 1     , fp );
fread( values[0]      , sizeof(gfloat) , 12*31 , fp );
fread( flags[0]       , sizeof(char)   , 12*31 , fp );

   if ( byte_order != G_BYTE_ORDER )
   {
      year = GINT16_FROM_LE(year);
      for ( i=0 ; i<12 ; i++ )
         for ( j=0 ; j<31 ; j++ )
            values[i][j] = gfloat_from_le(values[i][j]);
   }

   g_memmove(rec->staname,station_number,7);
   rec->year = year;
   rec->parameter = parameter;
   for ( i=0 ; i<12 ; i++ )
      for ( j=0 ; j<31 ; j++ )
         rec->flows[i][j] = values[i][j];

   g_free( values[0] );
   g_free( values );
   g_free( flags[0] );
   g_free( flags );
   g_free( station_number );

   return 0;
}
