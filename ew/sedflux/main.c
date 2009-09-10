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

#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "sedflux.h"
#include "sedflux_api.h"

void
print_sedflux_var (Sedflux_state* s, const gchar* prefix, const gchar* val_s)
{
  eh_require (s);
  eh_require (prefix);
  eh_require (val_s);

  {
    double* z;
    const int nx = sedflux_get_nx (s);
    const int ny = sedflux_get_ny (s);
    gint len[3] = {1, nx, ny};
    double size[3] = {1, 1, 1};

    z = sedflux_get_value (s, val_s, len);
    //z = sedflux_get_value_cube (s, val_s, len);

    eh_bov_print (prefix, z, val_s, len, size, NULL);

    eh_free (z);
  }
}

int
main (int argc, char *argv[])
{
  GError* error       = NULL;
  gchar*  command_str = NULL;
  gchar* init_file = NULL;
  gchar* work_dir = NULL;
  gchar* run_desc = NULL;
  gint dimen = 0;

  g_thread_init (NULL);
  eh_init_glib ();
  g_log_set_handler (NULL, G_LOG_LEVEL_MASK, &eh_logger, NULL);

  { /* Parse command line arguments */
    Sedflux_param_st* p = NULL;

    command_str = eh_render_command_str (argc, argv);

    p = sedflux_parse_command_line (argc, argv, &error);
    eh_exit_on_error( error , "Error parsing command line arguments" );

    init_file = g_strdup (p->init_file);
    work_dir = g_strdup (p->working_dir);
    run_desc = g_strdup (p->run_desc);
    dimen = p->mode_2d?(2):(3);

    eh_free( p );
  }

  { /* Create the project directory and check permissions */
    sedflux_setup_project_dir( &init_file , &work_dir , &error );
    eh_exit_on_error( error , "Error setting up project directory" );

    sedflux_print_info_file (init_file, work_dir, command_str, run_desc);
  }

   /* Setup the signal handling */
  sed_signal_set_action();

  { /* Initialze sedflux and then run it. */
    Sedflux_state* state = sedflux_initialize (init_file, dimen);

    if (state)
    {
      double start = sedflux_get_start_time (state);
      double end = sedflux_get_end_time (state);
      double dt = (end-start)/100;
      double t;
      gchar* prefix = NULL;
      int i;

      for (t=start+dt, i=0; t<end; t+=dt, i++)
      {
        sedflux_run_until (state, t);

/*
        prefix = g_strdup_printf ("%s_%.4d","grain",i);
        print_sedflux_var (state, prefix, "Elevation");
        //print_sedflux_var (state, prefix, "grain");

        eh_free (prefix);
*/
      }
    }

    sedflux_finalize (state);
  }

  if (g_getenv("SED_MEM_CHECK"))
    eh_heap_dump( "heap_dump.txt" );

  eh_exit (EXIT_SUCCESS);

  return EXIT_SUCCESS;
}

#if defined(OLD_WAY)
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
#endif

