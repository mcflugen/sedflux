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
" create_hydro - create a hydrotrend file from         ",
"                discharge data.                       ",
"                                                      ",
" options:                                             ",
"  in    : name of the input file.                     ",
"  v     : be verbose [no]                             ",
"                                                      ",
NULL };

Sed_hydro discharge_to_hydro_record( double q );

#define FORGET_THIS_FOR_NOW
int main(int argc,char *argv[])
{
#if !defined(FORGET_THIS_FOR_NOW)
   char *req[] = { "in" , NULL };
   char *infile;
   gboolean verbose;
   Eh_args *args;
   int i, n_cols, n_rows;
   double *row;
   double novalue;
   Eh_data_file *fp_in;
   Eh_data_record *data;
   Symbol_table *header;
   Sed_hydro rec;

   args = eh_opts_init(argc,argv);
   if ( eh_check_opts( args , req , NULL , help_msg )!=0 )
      eh_exit(-1);

   verbose = eh_get_opt_bool( args , "v"  , FALSE );
   infile  = eh_get_opt_str ( args , "in" , NULL  );

   fp_in = eh_open_data_file( infile , NULL );

   data = eh_get_data_from_file( fp_in , 0 );

   header = eh_get_data_record_sym_table( data );

   novalue = g_strtod( eh_symbol_table_lookup(header,"no value") , NULL );

   n_rows = eh_get_data_record_size( data , 0 );
   n_cols = eh_get_data_record_size( data , 1 );

   row = eh_get_data_record_row_ptr( data , 0 , double );
   for ( i=0 ; i<n_cols ; i++ )
   {
      if ( fabs(row[i]-novalue) > 1e-5 )
      {
         rec = discharge_to_hydro_record( row[i] );
         fprintf(stderr,"%f , %f\n",row[i],rec->velocity);
         hydro_destroy_hydro_record( rec );
      }
   }
#endif
   return 0;
}

#undef FORGET_THIS_FOR_NOW

Sed_hydro discharge_to_hydro_record( double q )
{
   Sed_hydro rec = sed_hydro_new( 1 );
   double a, b, c, d, e, f;
   
   a = c = e = 1.;
   b = d = f = 1/3.;
   
   sed_hydro_set_velocity         ( rec , a*pow(q,b) );
   sed_hydro_set_width            ( rec , c*pow(q,d) );
   sed_hydro_set_depth            ( rec , e*pow(q,f) );
   sed_hydro_set_bedload          ( rec , 0.         );
   sed_hydro_set_nth_concentration( rec , 0 , 0.     );

   return rec;
}

