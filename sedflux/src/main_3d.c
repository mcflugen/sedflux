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

// Author:
//
//  Eric Hutton
//  Institute of Arctic and Alpine Research
//  University of Colorado at Boulder
//  Boulder, CO
//  80309-0450
//
// Name :
//
// Synopsis :
//
// int               main                             ( int argc,
//                                                      char *argv[] )
// int               print_header                     ( FILE *fp )
// int               print_time                       ( FILE *fp,
//                                                      int epoch_no,
//                                                      double year )
// int               print_footer                     ( FILE *fp )
// void              print_choices                    ( int i )
//
// Description :
//


#include <stdio.h>
#include <limits.h>
#include <signal.h>
#include <time.h>

#include "utils.h"
#include "eh_project.h"
#include "sed_sedflux.h"
#include "processes.h"

extern char *help_msg[];
extern int __quit_signal;
extern int __dump_signal;
extern int __cpr_signal;

char *get_file_name_interactively( char** , char** );
char *eh_get_input_val( FILE *fp , char *msg , char *default_str );
void print_choices(int);
Sed_cube *init_cube(GSList *symbol_table_list , const char *img_file );
GSList *init_epoch_list( GSList*symbol_table_list );
int print_header(FILE *fp);
int print_time(FILE *fp, int epoch_no, double year);
int print_footer(FILE *fp);
Sed_process_list *create_process_list( void );
gboolean check_process_files_3( GSList *e_list );

//int main_2d(int argc, char *argv[]);
int run_sedflux(int argc, char *argv[]);
int main_3d(int argc, char *argv[]);

int main( int argc , char *argv[] )
{
   char *program_name;

   eh_init_glib();

   program_name = g_path_get_basename( argv[0] );

   if ( strcasecmp( program_name , "sedflux3d" )==0 )
   {
      g_set_application_name( "sedflux3d" );
      run_sedflux( argc , argv );
   }
   else if (    strcasecmp( program_name , "sedflux2d" )==0 
             || strcasecmp( program_name , "sedflux"   )==0 )
   {
      g_set_application_name( "sedflux2d" );
      run_sedflux( argc , argv );
   }
   else
      eh_exit(-1);

   return 1;
}

gboolean is_sedflux_3d( )
{
   if ( strcasecmp( g_get_application_name( ) , "sedflux3d" )==0 )
      return TRUE;
   else
      return FALSE;
}

