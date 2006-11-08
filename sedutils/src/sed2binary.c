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

#include <math.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "sed_sedflux.h"

/*** Self Documentation ***/
char *help_msg[] =
{
"                                                                             ",
" sed2binary [options] [parameters] [filein]                                  ",
"  convert a sedflux property output file into flat binary format.            ",
"                                                                             ",
" Options                                                                     ",
"  -v          - be verbose. [off]                                            ",
"  -h          - print this help message.                                     ",
"  -hfile      - print a help message on the file formats.                    ",
"                                                                             ",
NULL
};

#define FORGET_THIS_FOR_NOW

int main(int argc, char *argv[])
{
#if !defined( FORGET_THIS_FOR_NOW )
   Sed_file *fpin;
   FILE *fpout;
   char *infile, *outfile;
   char *verbose;
   Profile_header *hdr;
   double **data;
   Eh_args *args;

   args = eh_opts_init(argc,argv);
   if ( eh_check_opts( args , NULL , NULL , NULL )!=0 )
      eh_exit(-1);

   infile  = eh_get_opt_str( args , "in"      , NULL );
   outfile = eh_get_opt_str( args , "out"     , NULL );
   verbose = eh_get_opt_str( args , "verbose" , "no" );

   fpin = sed_open_sedflux_file_r( infile , NULL );
   sed_read_sedflux_profile( fpin );
   hdr = sed_get_sedflux_file_header( fpin );

   if ( !outfile )
      fpout = stdout;
   else
      if ( !(fpout=fopen(outfile,"w")) )
         perror(outfile), eh_exit(-1);

   if ( g_strcasecmp( verbose , "yes" )==0 )
   {
      eh_open_log(NULL);
      eh_print_log(DEFAULT_ERROR_LOG,"number of rows    : %d",hdr->n_rows);
      eh_print_log(DEFAULT_ERROR_LOG,"number of columns : %d",hdr->n_cols);
   }

   data = sed_get_sedflux_file_data( fpin );

   fwrite( data[0] , sizeof(double) , hdr->n_rows*hdr->n_cols , fpout );

   g_free(infile);
   g_free(outfile);

   sed_close_sedflux_file( fpin );
   fclose(fpout);
#endif

   return 0;
}

#undef FORGET_THIS_FOR_NOW

