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
" compact [options] [parameters]  [filein]                                    ",
"  compact a sedflux profile.                                                 ",
"                                                                             ",
" Options                                                                     ",
"  -v          - be verbose. [off]                                            ",
"  -h          - print this help message.                                     ",
"  -hfile      - print a help message on the file formats.                    ",
"  -a          - input will be ascii.  This is the default.                   ",
"  -b          - input will be a binary file from a sedflux dump.             ",
"                                                                             ",
" Parameters                                                                  ",
"  -pdy=value  - set the height of sediment cells in the output ASCII file to ",
"                be value.  The deafult is to not rebin the cells.            ",
"                                                                             ",
" Files                                                                       ",
"  -fsed=file  - specify the name of the file containing the sediment         ",
"                information. [compact.sed]                                   ",
"  -fin=file   - specify the name of the input file. [stdin]                  ",
"  -fout=file  - specify the name of the output file. [stdout]                ",
"                                                                             ",
NULL
};

char *file_msg[] =
{
"                                                                             ",
" The input file consists of a header line followed by data lines.  The header",
" line consists of a single number stating the number of data lines to follow.",
" For each cell in the sediment column that is to be compacted, the input file",
" gives the thickness of a cell and the fractions of each grain type that make",
" up that cell.  Each line of the file describes packages of sediment with the",
" first being the top of the sediment column.  The lines are divided into at  ",
" least two columns.  The first is the thickness (in meters) of that sediment ",
" package, and the rest are the fractions of each grain that compose that     ",
" cell.  The fractions are renormalized and so don't have to add up to one.   ",
"                                                                             ",
NULL
};

Sed_column read_sediment_column(FILE *fp,double dy);
int write_sediment_column(Sed_column s, FILE *fp);
int compact( Sed_column col );

#define COMPACTION_NAME             "compact"
#define COMPACTION_MAJOR_VERSION    "1"
#define COMPACTION_MINOR_VERSION    "0"

#define DEFAULT_CELL_HEIGHT         (1.)
#define DEFAULT_REBIN               (FALSE)
#define DEFAULT_ASCII               (TRUE)
#define DEFAULT_VERBOSE             (FALSE)
#define DEFAULT_IN_FILE             stdin
#define DEFAULT_OUT_FILE            stdout
#define DEFAULT_IN_FILE_NAME        "stdin"
#define DEFAULT_OUT_FILE_NAME       "stdout"
#define DEFAULT_SED_FILE_NAME  "compact.sed"

static Eh_opt_entry entries[] =
{
   { "cell-height" , 'h' , "Cell thickness"               , "VAL"  , "1.0" } ,
   { "rebin"       , 'r' , "Rebin cells after compaction" , NULL   , "FALSE" } ,
   { "ascii"       , 'a' , "Read from ascii file"         , NULL   , "TRUE" } ,
   { "verbose"     , 'v' , "Be verbose"                   , NULL   , "FALSE" } ,
   { "infile"      , 0   , "Input file"                   , "file" , "stdin" } ,
   { "outfile"     , 0   , "Output file"                  , "file" , "stdout" } ,
   { "sedfile"     , 0   , "Sediment file"                , "file" , "DEFAULT" } ,
   {NULL}
};

int main(int argc, char *argv[])
{
   Eh_opt_context opt;
   FILE *fpin, *fpout;
   char *infile, *outfile, *sedfile;
   gboolean verbose, ascii, rebin;
   double dy;
   Sed_column col;

   eh_init_glib();

   fprintf(stderr,"welcome to %s-%s.%s.  %s -h for help.\n",COMPACTION_NAME,COMPACTION_MAJOR_VERSION,COMPACTION_MINOR_VERSION,COMPACTION_NAME);
   fflush( stderr );

   opt = eh_opt_create_context( "compact" ,
                                "Sediment compaction model" ,
                                "Show compaction options" );
   opt = eh_opt_set_context( opt , entries );
   eh_opt_parse_context( opt , &argc , &argv , NULL );

   infile  = eh_opt_str_value( opt , "infile" );
   outfile = eh_opt_str_value( opt , "outfile" );
   sedfile = eh_opt_str_value( opt , "sedfile" );
   dy      = eh_opt_dbl_value( opt , "cell-height" );
   verbose = eh_opt_bool_value( opt , "verbose" );
   ascii   = eh_opt_bool_value( opt , "ascii" );
   rebin   = eh_opt_bool_value( opt , "rebin" );

   if ( strcmp( infile , "stdin" )==0 )
      fpin = stdin;
   else
      fpin = eh_fopen(infile,"r");
   if ( strcmp( outfile , "stdout" )==0 )
      fpout = stdout;
   else
      fpout = eh_fopen(outfile,"w");
   if ( strcmp( sedfile , "DEFAULT" )==0 )
      sedfile = NULL;

   if ( ascii )
   {
      GError*      error = NULL;
      Sed_sediment sed   = sed_sediment_scan( sedfile , &error );

      if ( !sed )
         eh_error( "%s: Unable to read sediment file: %s" , sedfile , error->message);

      sed_sediment_set_env( sed );

      col = read_sediment_column(fpin,dy);

      sed_sediment_destroy( sed );
   }
   else
      eh_require_not_reached();

   if ( verbose )
      eh_message("Initial mass = %f\n",sed_column_mass(col));

   eh_debug( "Compact the column." );
   compact( col );

   if ( rebin )
      sed_column_rebin(col);

   if ( verbose )
      eh_message("Final   mass = %f\n",sed_column_mass(col));

   eh_debug( "Write the output bulk densities." );
   if ( ascii )
      write_sediment_column(col,fpout);
   else
      eh_require_not_reached();

   sed_column_destroy(col);
   sed_sediment_unset_env();

   fclose(fpout);
   fclose(fpin);

   return 0;
}

Sed_column read_sediment_column(FILE *fp,double dy)
{
   Sed_column s;
   gssize n_cells, n_grains;
   double **fraction;
   double *thickness;

   eh_require( fp     );
   eh_require( dy > 0 );

   n_grains = sed_sediment_env_n_types();

   fscanf(fp,"%d\n",&n_cells);

   eh_require( n_cells > 0 );

   s = sed_column_new( n_cells );
   sed_column_set_z_res( s , dy );

   fraction  = eh_new_2( double , n_cells , n_grains );
   thickness = eh_new  ( double , n_cells );

   eh_debug( "Read sediment cells from input files." );
   {
      gssize i, n;
      for (i=n_cells-1;i>=0;i--)
      {
         fscanf(fp,"%lf",&thickness[n_cells-1-i]);
         for (n=0;n<n_grains;n++)
            if ( fscanf(fp,"%lf",&fraction[n_cells-1-i][n]) != 1 )
               eh_error( "Not enough columns in input file.\n");
         fscanf(fp,"\n");
      }
   }

   eh_debug( "Fill the sediment column with sediment" );
   {
      Sed_cell c = sed_cell_new_sized(n_grains,dy,fraction[0]);
      gssize i;

      for ( i=0 ; i<n_cells ; i++ )
      {
         sed_cell_set_fraction( c , fraction[i] );
         sed_column_add_cell( s , c );
      }

      for ( i=0 ; i<n_cells ; i++ )
         sed_column_resize_cell( s , i , thickness[i] );

      sed_cell_destroy( c );
   }

   eh_free_2( fraction  );
   eh_free  ( thickness );

   return s;
}

int write_sediment_column( Sed_column s , FILE *fp )
{
   gssize i, n;
   Sed_cell this_cell;
   gssize n_grains = sed_sediment_env_n_types();

   eh_require( s  );
   eh_require( fp );

   fprintf(fp,"%d\n",sed_column_len(s));
   for (i=sed_column_len(s)-1;i>=0;i--)
   {
      this_cell = sed_column_nth_cell( s , i );
      fprintf(fp,"%f ",sed_cell_size(this_cell));
      for ( n=0 ; n<n_grains ; n++ )
         fprintf(fp,"%f ",sed_cell_nth_fraction(this_cell,n));
      fprintf(fp,"\n");
   }

   return 0;
}

