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
#include "utils.h"
#include "sed_sedflux.h"

static char *help_msg[] = {
" read_hydro - read a hydrotrend output file.          ",
"                                                      ",
" options:                                             ",
"  nrecs : the number of records to read  [10]         ",
"  len   : the length of the buffer [365]              ",
"  nsig  : the number of significant events [10]       ",
"  start : indicate the record to start on [0]         ",
"  prop  : indicate the property to read [q]           ",
"        : q   = water flux                            ",
"        : qs  = suspended sediment flux               ",
"        : v   = river velocity                        ",
"        : w   = river width                           ",
"        : d   = river depth                           ",
"        : bed = bedload flux                          ",
"  v     : be verbose [no]                             ",
"  buf   : use a buffer to process the input file [no] ",
"  in    : name of the input file [stdin]              ",
"                                                      ",
NULL };

int main(int argc,char *argv[])
{
   char *prop_vals[] = { "q" , "qs" , "v" , "w" , "d" , "bed" , NULL };
   int j;
   int n_recs, buf_len, n_sig;
   int start;
   int type;
   double time, total_time;
   int prop;
   gboolean verbose, use_buf;
   char *infile;
   Eh_args *args;
   Hydro_get_val_func get_val;
   Sed_hydro_file fp;
   Sed_hydro rec;
   GPtrArray *rec_array;

   args = eh_opts_init(argc,argv);
   if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
      eh_exit(-1);

   total_time = eh_get_opt_dbl ( args , "dt" , 365.    );
   n_recs  = eh_get_opt_int ( args , "nrecs" , 10    );
   buf_len = eh_get_opt_int ( args , "len"   , 365   );
   n_sig   = eh_get_opt_int ( args , "nsig"  , 5     );
   start   = eh_get_opt_int ( args , "start" , 0     );
   verbose = eh_get_opt_bool( args , "v"     , FALSE );
   use_buf = eh_get_opt_bool( args , "buf"   , FALSE );
   infile  = eh_get_opt_str ( args , "in"    , "-"   );
   prop    = eh_get_opt_key ( args , "prop"  , 0 , prop_vals );
   
   switch ( prop )
   {
      case 0:
         get_val = &sed_hydro_water_flux;
         break;
      case 1:
         get_val = &sed_hydro_suspended_flux;
         break;
      case 2:
         get_val = &sed_hydro_velocity;
         break;
      case 3:
         get_val = &sed_hydro_width;
         break;
      case 4:
         get_val = &sed_hydro_depth;
         break;
      case 5:
         get_val = &sed_hydro_bedload;
         break;
   }
   
   if ( use_buf )
      type = HYDRO_HYDROTREND|HYDRO_USE_BUFFER;
   else
      type = HYDRO_HYDROTREND;

   if ( verbose )
   {
      fprintf(stderr,"--- head ---\n");
      fprintf(stderr,"total time (days) : %f\n",total_time);
      fprintf(stderr,"n records    : %d\n",n_recs);
      fprintf(stderr,"buf length   : %d\n",buf_len);
      fprintf(stderr,"n sig events : %d\n",n_sig);
      fprintf(stderr,"start        : %d\n",start);
      fprintf(stderr,"property     : %s\n",prop_vals[prop]);
      fprintf(stderr,"buffer       : %d\n",use_buf);
   }

   fp = sed_hydro_file_new( infile , type , TRUE );
   sed_hydro_file_set_sig_values( fp , n_sig );
   sed_hydro_file_set_buffer_length( fp , buf_len );

   rec_array = g_ptr_array_new( );
   fprintf(stdout,"%s\n",prop_vals[prop]);
   for ( time=0 ; time<total_time ; )
   {
      rec = sed_hydro_file_read_record( fp );
      if ( sed_hydro_duration(rec) + time > total_time )
         sed_hydro_set_duration( rec , total_time-time );
      for ( j=0 ; j<sed_hydro_duration(rec) ; j++ )
         fprintf(stdout,"%f\n",(*get_val)( rec ));
      time += sed_hydro_duration(rec);
      g_ptr_array_add( rec_array , rec );
   }
   
   if ( verbose )
   {
      double total = 0, total_qs = 0;

      for ( j=0 ; j<rec_array->len ; j++ )
      {
         rec       = g_ptr_array_index( rec_array , j );
         total    += (*get_val)( rec )*sed_hydro_duration(rec);
         total_qs += sed_hydro_suspended_load( rec );
      }

      fprintf(stderr,"--- tail ---\n");
      fprintf(stderr,"n events         : %d\n",rec_array->len);
      fprintf(stderr,"total            : %f\n",total);
      fprintf(stderr,"total qs         : %f\n",total_qs);
   }

   g_ptr_array_free( rec_array , FALSE );

   sed_hydro_file_destroy( fp );

   return 0;
}

