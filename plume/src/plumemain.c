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

#include "plumevars.h"
#include "plumeinput.h"

#include "glib.h"
#include "utils.h"

/*** Self Documentation ***/
char *help_msg[] =
{
"                                                                             ",
" plume [options] [parameters]                                                ",
"  model a 2D sediment plume using the Albertson model.                       ",
"                                                                             ",
" Options                                                                     ",
"                                                                             ",
"  -o outfile - use outfile for output file prefixes. [outfile]               ",
"  -1         - assume the plume centerline is 1-D (ie it doesn't bend).      ",
"               [default]                                                     ",
"  -2         - allow the plume centerline to bend in 2-D.                    ",
"  -v         - be verbose. [off]                                             ",
"  -h         - print this help message.                                      ",
"                                                                             ",
" Parameters                                                                  ",
"                                                                             ",
"  MODEL PARAMETERS                                                           ",
"   These options set constants that are used in the model.                   ",
"   -pu0=value - Set the river mouth velocity to value.                       ",
"   -pQ=value  - Set river discharge to value.                                ",
"                                                                             ",
"  FILES                                                                      ",
"   These options specify which files data is output to.                      ",
"   -fccnc=file                                                               ",
"   -fncnc=file                                                               ",
"   -fdeps=file                                                               ",
"   -fualb=file                                                               ",
"   -fsln=file                                                                ",
"                                                                             ",
NULL
};

#define DEFAULT_VERBOSE       (FALSE)
#define DEFAULT_OUT_FILE      stdout
#define DEFAULT_IN_FILE       stdin
#define DEFAULT_OUT_FILE_NAME "outfile"
#define DEFAULT_IN_FILE_NAME  "stdin"
#define DEFAULT_RIVER_Q       (-1)
#define DEFAULT_RIVER_U0      (-1)
#define DEFAULT_FJORD         (-1)

int plumeout1( char *filename , Plume_enviro* , Plume_grid* );
int plumeread( FILE* , Plume_enviro* , Plume_grid* , Plume_options* );

int main(int argc, char *argv[])
{
   Eh_args *args;
   FILE *fpin, *fpout;
   char *infile, *outfile;
   gboolean verbose;
   int fjord;
   double river_q, river_u0;
   int err;
   FILE *fidlog;
   Plume_enviro env;
   Plume_grid grid;
   Plume_options opt;
   Plume_mass_bal mb;

   args = eh_opts_init( argc , argv );
   if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
      eh_exit( -1 );

   infile   = eh_get_opt_str ( args , "in"  , DEFAULT_IN_FILE_NAME  );
   outfile  = eh_get_opt_str ( args , "out" , DEFAULT_OUT_FILE_NAME );
   river_q  = eh_get_opt_dbl ( args , "q"   , DEFAULT_RIVER_Q       );
   river_u0 = eh_get_opt_dbl ( args , "u0"  , DEFAULT_RIVER_U0      );
   fjord    = eh_get_opt_int ( args , "f"   , DEFAULT_FJORD         );
   verbose  = eh_get_opt_bool( args , "v"   , DEFAULT_VERBOSE       );
   
   fpin  = stdin;
   fpout = stdout;
   if ( strcmp( infile , "stdin" )!=0 )
      fpin = eh_fopen(infile,"r");
   if ( strcmp( outfile , "stdout" )!=0 )
      fpout = eh_fopen(outfile,"w");

   env.river = eh_new( Plume_river , 1 );
   env.ocean = eh_new( Plume_ocean , 1 );
   env.sed   = eh_new( Plume_sediment , 1 );
   
   /* Read plume constants from a file. */
   err = plumeread(fpin , &env , &grid , &opt );

   if ( river_q != -1 )
      env.river->Q = river_q;
   if ( river_u0 != -1 )
      env.river->u0 = river_u0;
   if ( fjord != -1 )
      opt.fjrd = fjord;

   if( verbose ) {
      if( (fidlog = fopen("plume.log","a+")) == NULL)
         printf("PLUME ERROR: Unable to open the log file, plume.log");

      fprintf(fidlog," ======================================================== \n\n");
      fprintf(fidlog," ----- PLUME Model Run ----- \n\n");
      fflush(fidlog);
   }

#ifdef DBG
   if( !verbose )
      if( (fidlog = fopen("plume.log","a+")) == NULL)
         printf("PLUME ERROR: Unable to open the log file, plume.log");

      fprintf(fidlog," ======================================================== \n\n");
      fprintf(fidlog," ----- PLUME Model Run ----- \n\n");
#endif

   plume( &env , &grid , &opt );

   err = plumeout1( outfile , &env , &grid );

   if ( verbose )
      err = plumelog( &env , &grid , &opt , &mb );

   fclose(fpin);
   fclose(fpout);

   return 0;
}

