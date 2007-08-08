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
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "utils.h"
#include "eh_project.h"
#include "sed_sedflux.h"
#include "sedflux.h"

int
main( int argc , char *argv[] )
{
   gboolean          success     = TRUE;
   Sedflux_param_st* p           = NULL;
   GError*           error       = NULL;
   gchar*            command_str = NULL;

   g_thread_init( NULL );
   eh_init_glib();
   g_log_set_handler( NULL , G_LOG_LEVEL_MASK , &eh_logger , NULL );

   command_str = eh_render_command_str( argc , argv );

   /* Parse command line arguments */
   p = sedflux_parse_command_line( argc , argv , &error );
   eh_exit_on_error( error , "Error parsing command line arguments" );

   /* Create the project directory and check permissions */
   sedflux_setup_project_dir( &p->init_file , &p->working_dir , &error );
   eh_exit_on_error( error , "Error setting up project directory" );

   sedflux_print_info_file( p->init_file , p->working_dir , command_str , p->run_desc );

   /* Setup the signal handling */
   sed_signal_set_action();

   /* Run sedflux!!! */
   success = sedflux( p->init_file );

   eh_free( p );

   if ( g_getenv("SED_MEM_CHECK") ) eh_heap_dump( "heap_dump.txt" );

   if ( success ) eh_exit( EXIT_SUCCESS );
   else           eh_exit( EXIT_FAILURE );

   return EXIT_SUCCESS;
}


