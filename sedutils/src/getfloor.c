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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include "utils.h"
#include "sed_sedflux.h"

int main(int argc,char *argv[])
{
   char *rec[] = { "in" , "dx" , "len" , NULL };
   FILE *fpout;
   char *infile, *outfile;
   double *x, *d;
   double dx, len;
   int i, n;
   Eh_args *args;

   args = eh_opts_init(argc,argv);
   if ( eh_check_opts( args , rec , NULL , NULL )!=0 )
      eh_exit(-1);

   dx      = eh_get_opt_dbl( args , "dx"  , -1   );
   len     = eh_get_opt_dbl( args , "len" , -1   );
   infile  = eh_get_opt_str( args , "in"  , NULL );
   outfile = eh_get_opt_str( args , "out" , NULL );

   if ( !outfile )
      fpout = stdout;
   else
      if ( !(fpout=fopen(outfile,"w")) )
         perror(outfile), eh_exit(-1);

   n = (int) (len / dx);

   x = g_new0( double , n );
   d = g_new0( double , n );

   for (i=0;i<n;i++)
      x[i] = dx*i;

   sed_get_floor_vec(infile,x,n,d);

   fwrite(d,n,sizeof(double),fpout);

   free(x);
   free(d);
   g_free(infile);
   g_free(outfile);

   return 0;
}

