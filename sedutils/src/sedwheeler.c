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
#include "utils.h"
#include "sed_sedflux.h"

#define DEFAULT_T0 (0)
#define DEFAULT_T1 (-1)

#define WHEELER_EROSION    (0)
#define WHEELER_HIATUS     (1)
#define WHEELER_DEPOSITION (2)

#define EROSION_DEPOSITION (0)
#define SEDIMENTATION_RATE (1)
#define TIME_LINE          (2)

double get_type_val( double z_0 , double z_1 , double z_min , int type );
double get_rate_val( double z_0 , double z_1 , double z_min , int type );
double get_time_line_val( double z_0 , double z_1 , double z_min , int type );

#define FORGET_THIS_FOR_NOW

int main( int argc , char *argv[] )
{
#if !defined(FORGET_THIS_FOR_NOW)
   gint64 i, j;
   gint64  vals_per_rec, n_recs, n_cols;
   int type = EROSION_DEPOSITION;
   int start, end;
   int rec_0, rec_1;
   FILE *fp_in, *fp_out;
   char *str = g_new( char , 2048 );
   char *infile, *outfile;
   gboolean verbose;
   double t_0, t_1;
   double *t, *x, **d;
   double d_min;
   Eh_args *args;
   Met_station_header hdr;
   double (*get_val)(double,double,double,int);

   args = eh_opts_init(argc,argv);
   if ( eh_check_opts( args , NULL , NULL , NULL )!=0 )
      eh_exit(-1);

   infile     = eh_get_opt_str ( args , "in"      , NULL       );
   outfile    = eh_get_opt_str ( args , "out"     , NULL       );
   t_0        = eh_get_opt_dbl ( args , "t0"      , DEFAULT_T0 );
   t_1        = eh_get_opt_dbl ( args , "t1"      , DEFAULT_T1 );
   verbose    = eh_get_opt_bool( args , "verbose" , FALSE      );

   if ( infile )
   {
      fp_in = fopen( infile , "r" );
      if ( !fp_in )
         perror( infile );
   }
   else
      fp_in = stdin;
   if ( outfile )
   {
      fp_out = fopen( outfile , "w" );
      if ( !fp_out )
         perror( outfile );
   }
   else
      fp_out = stdout;

   hdr = sed_read_measurement_header( fp_in );

   if ( verbose )
      sed_write_measurement_header( stderr , &hdr );

   fread( &vals_per_rec , sizeof(gint64) , 1 , fp_in );

   n_cols = (vals_per_rec-1)/2;

   start = ftell( fp_in );
   fseek( fp_in , 0 , SEEK_END );
   end   = ftell( fp_in );
   n_recs = (end-start)/(sizeof(double)*vals_per_rec);
   fseek( fp_in , start , SEEK_SET );

   t = g_new( double  , n_recs );
   x = g_new( double  , n_cols );
   d = g_new( double* , n_recs );

   for ( i=0 ; i<n_recs ; i++ )
   {
      d[i] = g_new( double , n_cols );
      fread( &(t[i]) , sizeof(double) , 1 , fp_in );
      fread( x       , sizeof(double) , n_cols , fp_in );
      fread( d[i]    , sizeof(double) , n_cols , fp_in );
   }

   for ( rec_0=0        ; rec_0<n_recs && t[rec_0]<t_0 ; rec_0++ );
   for ( rec_1=n_recs-1 ; rec_1>=0     && t[rec_1]>t_1 ; rec_1-- );

   if ( t_0<0 )
      rec_0 = 0;
   if ( t_1<0 )
      rec_1 = n_recs-1;

   g_free( hdr.parameter );

   if ( type == EROSION_DEPOSITION )
   {
      get_val = &get_type_val;
      hdr.parameter = g_strdup( "erosion / deposition" );
   }
   else if ( type == SEDIMENTATION_RATE )
   {
      get_val = &get_rate_val;
      hdr.parameter = g_strdup( "sedimentation rate" );
   }
   else if ( type == TIME_LINE )
   {
      get_val = &get_time_line_val;
      hdr.parameter = g_strdup( "sediment thickness" );
   }
   else
      eh_require_not_reached();

   for ( i=0 ; i<n_cols ; i++ )
   {
      for ( j=rec_1 ; j>=rec_0+1 ; j-- )
      {
         if ( d[j][i]<d[j-1][i] )
         {
            d_min = d[j][i];
            for ( ; j>=1 && d[j-1][i]>d_min ; j-- )
               d[j][i] = (*get_val)( d[j-1][i] , d[j][i] ,
                                  d_min     , WHEELER_EROSION );
/*
               if ( d[j-1][i]-d_min<1e-5 )
                  d[j][i] = WHEELER_HIATUS;
//                  d[j][i] = d_min - d[j-1][i];
//                  d[j][i] = d_min;
               else
                  d[j][i] = WHEELER_EROSION;
//                  d[j][i] = d_min - d[j-1][i];
//                  d[j][i] = d_min;
*/
            j++;
         }
         else if ( d[j][i]-d[j-1][i]>.1 )
            d[j][i] = (*get_val)( d[j-1][i] , d[j][i] ,
                               d_min     , WHEELER_DEPOSITION );
/*
            d[j][i] = WHEELER_DEPOSITION;
//            d[j][i] = d[j][i]-d[j-1][i];
//            d[j][i] = d[j][i];
*/
         else
            d[j][i] = (*get_val)( d[j-1][i] , d[j][i] ,
                               d_min     , WHEELER_HIATUS );
/*
            d[j][i] = WHEELER_HIATUS;
//            d[j][i] = d[j][i]-d[j-1][i];
//            d[j][i] = d[j][i];
*/
      }
      d[j][i] = (*get_val)( d[j][i] , d[j][i] ,
                         d_min   , WHEELER_HIATUS );
/*
      d[0][i] = WHEELER_HIATUS;
//      d[0][i] = 0;
//      d[0][i] = d[0][i];
*/
   }

   sed_write_measurement_header( fp_out , &hdr );
   fwrite( &vals_per_rec , sizeof(gint64) , 1 , fp_out );
   for ( i=rec_0 ; i<rec_1 ; i++ )
   {
      fwrite( &(t[i]) , sizeof(double) , 1      , fp_out );
      fwrite( x       , sizeof(double) , n_cols , fp_out );
      fwrite( d[i]    , sizeof(double) , n_cols , fp_out );
   }

   g_free( str );
#endif

   return 0;
}

#undef FORGET_THIS_FOR_NOW

double get_type_val( double z_0 , double z_1 , double z_min , int type )
{
   double rtn;
   if ( type == WHEELER_EROSION )
   {
      if ( z_0-z_min<1e-5 )
         rtn = WHEELER_HIATUS;
      else
         rtn = WHEELER_EROSION;
   }
   else if ( type == WHEELER_HIATUS )
      rtn = WHEELER_HIATUS;
   else if ( type == WHEELER_DEPOSITION )
      rtn = WHEELER_DEPOSITION;
   else
      eh_require_not_reached();

   return rtn;
}

double get_rate_val( double z_0 , double z_1 , double z_min , int type )
{
   double rtn;
   if ( type == WHEELER_EROSION )
      rtn = z_min - z_0;
   else if ( type == WHEELER_HIATUS )
      rtn = z_1 - z_0;
   else if ( type == WHEELER_DEPOSITION )
      rtn = z_1 - z_0;
   else
      eh_require_not_reached();

   return rtn;
}

double get_time_line_val( double z_0 , double z_1 , double z_min , int type )
{
   double rtn;
   if ( type == WHEELER_EROSION )
      rtn = z_min;
   else if ( type == WHEELER_HIATUS )
      rtn = z_1;
   else if ( type == WHEELER_DEPOSITION )
      rtn = z_1;
   else
      eh_require_not_reached();

   return rtn;
}

