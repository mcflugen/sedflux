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
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>

#define FORGET_THIS_FOR_NOW

#if !defined( FORGET_THIS_FOR_NOW )
/*** Self Documentation ***/
static char *help_msg[] =
{
"                                                                             ",
" sedextract [options] [parameters] [filein]                                  ",
"  extract a portion of a sedflux output file.                                ",
"                                                                             ",
" Options                                                                     ",
"  -b          - Extraction positions will be given by row and column numbers.",
"                This is the default.                                         ",
"  -m          - Extraction positions will be given in units of meters.       ",
"  out=outfile  - send output to outfile instead of stdout. [stdout]          ",
"  in=infile   - read input from infile rather than stdin. [stdin]            ",
"  verbose=yes - be verbose. [no]                                            ",
"  -h          - print this help message.                                     ",
"                                                                             ",
" Parameters                                                                  ",
"  x0=value  - Begin extraction at trace value. [0]                           ",
"  x1=value  - End extraction at trace value. [end]                           ",
"  y0=value  - Begin extraction at value value. [0]                           ",
"  y1=value  - End extraction at row value. [end]                             ",
"  nx=value  - Write out every value-th trace. [10]                           ",
"  ny=value  - Resample each trace using every value-th row. [1]              ",
NULL
};
#endif

#define UNITS_BINS   (0)
#define UNITS_METERS (1)

/* Default values */
#define DEFAULT_UNITS               UNITS_BINS
#define DEFAULT_X_START             (-1)
#define DEFAULT_X_STOP              (-1)
#define DEFAULT_Y_START             (-1)
#define DEFAULT_Y_STOP              (-1)
#define DEFAULT_X_INCREMENT         (10)
#define DEFAULT_Y_INCREMENT         (1)
#define DEFAULT_PRINT_HEADER        0
#define DEFAULT_VERBOSE             0
#define DEFAULT_IN_FILE             stdin
#define DEFAULT_OUT_FILE            stdout
#define DEFAULT_IN_FILE_NAME        "stdin"
#define DEFAULT_OUT_FILE_NAME       "stdout"

int main(int argc, char *argv[])
{

#if !defined( FORGET_THIS_FOR_NOW )
   double x0, x1, y0, y1;
   double nx, ny;
   char *units;
   char *header_flag;
   FILE *fpin, *fpout;
   char *infile, *outfile;
   char *verbose;
   Profile_header *header, *header_new;
   unsigned char **data;
   int i,j;
   int row, column;
   Eh_args *args;

   args = eh_opts_init(argc,argv);
   if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
      eh_exit(-1);

   x0          = eh_get_opt_dbl( args , "x0"      , DEFAULT_X_START );
   x1          = eh_get_opt_dbl( args , "x1"      , DEFAULT_X_STOP  );
   y0          = eh_get_opt_dbl( args , "y0"      , DEFAULT_Y_START );
   y1          = eh_get_opt_dbl( args , "y1"      , DEFAULT_Y_STOP  );
   nx          = eh_get_opt_dbl( args , "nx"      , DEFAULT_X_INCREMENT );
   ny          = eh_get_opt_dbl( args , "ny"      , DEFAULT_Y_INCREMENT );
   units       = eh_get_opt_str( args , "units"   , "bins" );
   header_flag = eh_get_opt_str( args , "header"  , "no" );
   verbose     = eh_get_opt_str( args , "verbose" , "no" );
   infile      = eh_get_opt_str( args , "in"      , NULL );
   outfile     = eh_get_opt_str( args , "out"     , NULL );

   if ( !infile )
      fpin = stdin;
   else
      if ( !(fpin=fopen(infile,"r")) )
         perror(infile), eh_exit(-1);
   if ( !outfile )
      fpout = stdout;
   else
      if ( !(fpout=fopen(outfile,"w")) )
         perror(outfile), eh_exit(-1);

// read the header
   header = sed_read_profile_header( fpin );

   if ( g_strcasecmp( header_flag , "yes" )==0 )
   {
      fprintf( stderr , "Number of rows    : %d\n"  , header->n_rows      );
      fprintf( stderr , "Number of columns : %ld\n" , header->n_cols      );
      fprintf( stderr , "Row height        : %f\n"  , header->cell_height );
      fprintf( stderr , "Column Width      : %f\n"  , header->cell_width  );
      fprintf( stderr , "Minimum value     : %f\n"  , header->min_value   );
      fprintf( stderr , "Maximum value     : %f\n"  , header->max_value   );
      
      eh_exit(0);
   }
   
   if ( g_strcasecmp( units , "meters" )==0 )
   {
      y0 /= header->cell_height;
      y1 /= header->cell_height;
      x0 /= header->cell_width;
      x1 /= header->cell_width;
/*
      ny /= header->cell_height;
      nx /= header->cell_width;
*/
   }
   else
      fprintf(stderr,"error : invalid unit argument, %s.\n",units), eh_exit(-1);

/* Read in the data */
   data = g_new( unsigned char* , header->n_cols );
   data[0] = g_new( unsigned char , header->n_cols*header->n_rows);
   for ( i=1 ; i<header->n_cols ; i++ )
      data[i] = data[i-1]+header->n_rows;
   fread( data[0] , sizeof(unsigned char) , header->n_rows*header->n_cols , fpin );

/* If x1 or y1 have not been specified, go to the end */
   if ( y1 < 0 ) y1 = header->n_rows;
   if ( x1 < 0 ) x1 = header->n_cols;

   if ( y1 > header->n_rows ) y1 = header->n_rows;
   if ( x1 > header->n_cols ) x1 = header->n_cols;
   
   if ( g_strcasecmp( verbose , "yes" )==0 )
   {
      fprintf(stderr,"sedextract : extracting image from (%f,%f) -> (%f,%f)\n",x0,y0,x1,y1);
      fprintf(stderr,"sedextract : resampling interval in the x-direction is %.0f\n",nx);
      fprintf(stderr,"sedextract : resampling interval in the y-direction is %.0f\n",ny);
   }

/* Write the new header */
   header_new = g_new( Profile_header , 1 );
   header_new->n_rows      = (int)((y1-y0)/ny+.5);
   header_new->n_cols      = (int)((x1-x0)/nx+.5);
   header_new->cell_height = header->cell_height*ny;
   header_new->cell_width  = header->cell_width*nx;
   header_new->max_value   = header->max_value;
   header_new->min_value   = header->min_value;
   sed_print_profile_header( fpout , header_new );

/* Write the data */
   for ( j=(int)x0,column=0 ; column<header_new->n_cols ; j+=(int)nx,column++ )
      for ( i=(int)y0,row=0 ; row<header_new->n_rows ; i+=(int)ny,row++ )
         fwrite(&data[j][i],sizeof(unsigned char),1,fpout);

   g_free(data);

   fclose(fpin);
   fclose(fpout);

#endif

   return 0;
}

#undef FORGET_THIS_FOR_NOW
