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

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <utils/utils.h>
#include <sed/sed_sedflux.h>

#include "my_sedflux.h"
#include "sedflux_api.h"

G_BEGIN_DECLS
gchar* sedflux_get_file_name_interactively (gchar **working_dir,
                                            gchar **in_file);
G_END_DECLS

struct _Sedflux_state
{
  Sed_epoch_queue q;
  Sed_cube p;

  char* init_file; //< Name of initialization file
  char* prefix; //< Path prefix for all input files
  char* working_dir; //< Path where sedflux will be run
  char* description; //< Short description of the simulation
  gboolean is_2d; //< Is sedflux to be run in 2d mode

  // Keep track of these variables so that we can take time derivatives
  double* thickness; //< Sediment thickness at the last time state
};

static Sed_process_init_t my_proc_defs[] =
{
   { "constants"         , init_constants     , run_constants   , destroy_constants } , 
   { "earthquake"        , init_quake         , run_quake       , destroy_quake     } , 
   { "tide"              , init_tide          , run_tide        , destroy_tide      } , 
   { "sea level"         , init_sea_level     , run_sea_level   , destroy_sea_level } ,
   { "storms"            , init_storm         , run_storm       , destroy_storm     } ,
   { "river"             , init_river         , run_river       , destroy_river     } ,
   { "erosion"           , init_erosion       , run_erosion     , destroy_erosion   } ,
   { "avulsion"          , init_avulsion      , run_avulsion    , destroy_avulsion  } ,

   /* A new process */
   { "new process"       , init_new_process   , run_new_process , destroy_new_process } ,

   /* The rest of the processes */
   { "bedload dumping"   , init_bedload       , run_bedload     , destroy_bedload     } ,
   { "plume"             , init_plume         , run_plume       , destroy_plume       } ,
   { "diffusion"         , init_diffusion     , run_diffusion   , destroy_diffusion   } ,
   { "xshore"            , init_xshore        , run_xshore      , destroy_xshore      } ,
   { "squall"            , init_squall        , run_squall      , destroy_squall      } ,
   { "bioturbation"      , bio_init           , bio_run         , bio_destroy } ,
   { "compaction"        , NULL               , run_compaction  , NULL                } ,
   { "flow"              , init_flow          , run_flow        , destroy_flow        } ,
   { "isostasy"          , init_isostasy      , run_isostasy    , destroy_isostasy    } ,
   { "subsidence"        , init_subsidence    , run_subsidence  , destroy_subsidence  } ,
   { "data dump"         , init_data_dump     , run_data_dump   , destroy_data_dump   } ,
   { "failure"           , init_failure       , run_failure     , destroy_failure     } ,
   { "measuring station" , init_met_station   , run_met_station , destroy_met_station } ,
   { "bbl"               , init_bbl           , run_bbl         , destroy_bbl         } ,
   { "cpr"               , init_cpr           , run_cpr         , destroy_cpr         } ,

   { "turbidity current" , init_inflow        , run_turbidity_inflow, destroy_inflow     } ,

   { "hypopycnal plume"  , init_plume_hypo    , run_plume_hypo  , destroy_plume_hypo  } ,
   { "inflow"            , init_inflow        , run_plume_hyper_inflow , destroy_inflow     } ,
   { "sakura"            , init_inflow        , run_plume_hyper_sakura , destroy_inflow     } ,

   { "debris flow"       , init_debris_flow   , run_debris_flow , destroy_debris_flow } ,
   { "slump"             , NULL               , run_slump       , NULL                } ,

   { NULL }
};

static Sed_process_family my_proc_family[] =
{
   { "failure" , "turbidity current" } ,
   { "failure" , "debris flow"       } ,
   { "failure" , "slump"             } ,
   { "plume"   , "hypopycnal plume"  } ,
   { "plume"   , "inflow"            } ,
   { "plume"   , "sakura"            } ,
   { NULL }
};

gboolean
sedflux (const int argc, const char* argv[])
{
  Sedflux_state* state = sedflux_initialize (argc, argv);

  eh_require (state)

  sedflux_run (state);
  sedflux_finalize (state);

  return TRUE;
}

#if 0
gboolean
old_sedflux (const gchar* init_file, const gchar* prefix, int dimen)
{
  Sedflux_state* state = sedflux_initialize (init_file, prefix, dimen);

  eh_require (state)

  sedflux_run (state);
  sedflux_finalize (state);

  return TRUE;
}
#endif

#if 0
void
_sedflux_set_up (const gchar* init_file, const gchar* input_dir,
                 const gint dimen, GError** error)
{
  eh_require (error==NULL || *error==NULL)
  eh_require (init_file);
  eh_require (dimen==2 || dimen==3);

  {
    GError* tmp_err = NULL;
    const int argc = 7;
    gchar* argv[argc];
    Sedflux_param_st* p = NULL;

    { /* Construct a sedflux command line */
      gchar* wdir = g_get_current_dir ();
      gchar* in_path = NULL;

      if (input_dir)
        in_path = g_strdup (input_dir);
      else
        in_path = g_strdup (".");

      argv[0] = "sedflux";
      argv[1] = (dimen==2)?"-2":"-3";
      argv[2] = g_strconcat ("--init-file=", init_file, NULL);
      argv[3] = g_strconcat ("--input-dir=", input_dir, NULL);
      argv[4] = g_strconcat ("--working-dir=", wdir, NULL);
      argv[5] = "--msg=\"Sedflux simulation\"";
      argv[6] = "--verbose";

      g_free (in_path);

      p = sedflux_parse_command_line (argc, argv, &tmp_err);

      eh_require (strcmp (p->init_file, init_file)==0);
      eh_require (strcmp (p->working_dir, wdir)==0);
      eh_require (p->mode_2d == (dimen==2));

      eh_free (wdir);
    }

    if (!tmp_err)
    { /*  Create the project directory and check permissions */
      gchar* command_str = eh_render_command_str (argc, argv);

      sedflux_setup_project_dir (&(p->init_file), &(p->input_dir),
                                 &(p->working_dir), &tmp_err);

      if (!tmp_err)
      {
        sedflux_print_info_file (p->init_file, p->working_dir,
                                 command_str, p->run_desc);
      }
      else
        g_propagate_error (error, tmp_err);

      eh_free (command_str);
    }
    else
      g_propagate_error (error, tmp_err);

    if (p)
    {
      eh_free (p->init_file);
      eh_free (p->working_dir);
      eh_free (p->run_desc);
      eh_free (p);
    }
  }

  return;
}
#endif


Sedflux_state*
sedflux_new (void)
{
  Sedflux_state* state = NULL;

  { /* Create a new state. */
    state = eh_new (Sedflux_state,1);

    state->q = NULL;
    state->p = NULL;
    state->init_file = NULL;
    state->prefix = NULL;
    state->working_dir = NULL;
    state->description = NULL;
    state->is_2d = TRUE;

    state->thickness = NULL;
  }

  return state;
}

Sedflux_state*
sedflux_set_init_file (Sedflux_state* self, gchar* init_file)
{
  eh_require (self);
  eh_require (init_file);

  if (self)
  {
    self->init_file = g_strdup (init_file);
  }

  return self;
}

Sedflux_state*
sedflux_set_input_dir (Sedflux_state* self, gchar* input_dir)
{
  eh_require (self);

  if (self)
  {
    if (input_dir)
      self->prefix = g_strdup (input_dir);
    else
      self->prefix = g_strdup (".");
  }

  return self;
}

Sedflux_state*
sedflux_set_working_dir (Sedflux_state* self, gchar* working_dir)
{
  eh_require (self);

  if (self)
  {
    if (working_dir)
      self->working_dir = g_strdup (working_dir);
    else
      self->working_dir = g_strdup (".");
  }

  return self;
}

Sedflux_state*
sedflux_set_description (Sedflux_state* self, gchar* description)
{
  eh_require (self);

  if (self)
  {
    if (description)
      self->description = g_strdup (description);
    else
      self->description = g_strdup ("");
  }

  return self;
}

Sedflux_state*
sedflux_set_dimension (Sedflux_state* self, gboolean is_2d)
{
  eh_require (self);

  if (self)
  {
    self->is_2d = is_2d;
    if (is_2d)
      sed_mode_set (SEDFLUX_MODE_2D);
    else
      sed_mode_set (SEDFLUX_MODE_3D);
  }
  eh_require (sed_mode_is_2d() || sed_mode_is_3d());

  return self;
}

/** Check that project directories are valid

Check to see that project directories are valid.  This means that they
exist and are readable and/or writable.  If an output directory does not
exist then we try to create it.  If any of the supplied project directories
are NULL, they are set to the current directory (".").

@param init_file Pointer to name of initialization file
@param input_dir Pointer to name of input directory
@param working_dir Pointer to name of output directory
@param error Pointer to a GError

@returns TRUE if everything went ok, FALSE otherwise (and @param error is
         set).
*/
gboolean
sedflux_set_project_dirs (Sedflux_state* self, GError** error)
{
  gboolean rtn_val = TRUE;
  GError* tmp_err = NULL;

  eh_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* If not given, prompt the user to supply the initialization file
     and working directory. */
  if (!self->init_file)
    sedflux_get_file_name_interactively (&(self->working_dir),
                                         &(self->init_file));

  if (!self->working_dir)
    self->working_dir = g_strdup (".");

  if (!self->prefix)
    self->prefix = g_strdup (".");

  /* Create the working directory */
  if (g_mkdir_with_parents (self->working_dir, 0)  == -1)
    eh_set_file_error_from_errno (&tmp_err, self->working_dir, errno);

  /* Make sure the working directory is writable */
  if (!tmp_err && g_access (self->working_dir, R_OK|W_OK|X_OK) == -1)
    eh_set_file_error_from_errno (&tmp_err, self->working_dir, errno);

  /* Move to the working directory */
  if (!tmp_err && g_chdir (self->working_dir) != 0)
    eh_set_file_error_from_errno (&tmp_err, self->working_dir, errno);

  /* Make sure there is an input directory */
  if (!tmp_err && !g_file_test (self->prefix, G_FILE_TEST_IS_DIR))
    eh_set_file_error_from_errno (&tmp_err, self->prefix, errno);

  {
    gchar* path = g_build_filename (self->prefix, self->init_file, NULL);

    /* Make sure the initialization file is readable */
    if (!tmp_err && g_access (path, R_OK) == -1)
      eh_set_file_error_from_errno (&tmp_err, path, errno);

    g_free (path);
  }

  if ( tmp_err )
  {
    g_propagate_error (error, tmp_err);
    rtn_val = FALSE;
  }

  return rtn_val;
}

const gchar*
sedflux_init_file (Sedflux_state* self)
{
  eh_require (self);

  return self->init_file;
}

const gchar*
sedflux_input_dir (Sedflux_state* self)
{
  eh_require (self);

  return self->prefix;
}

const gchar*
sedflux_working_dir (Sedflux_state* self)
{
  eh_require (self);

  return self->working_dir;
}

const gchar*
sedflux_description (Sedflux_state* self)
{
  eh_require (self);

  return self->description;
}

gchar*
sedflux_build_options (Sedflux_state* self)
{
  gchar * str = NULL;
/*
  { // Process order
    gchar *proc_list = sed_process_queue_names (my_proc_defs);
    str = g_strconcat ("Process order: ", proc_list, NULL);
    g_free (proc_list);
  }
*/
  { // Revision number
    gchar *proc_list = sed_process_queue_names (my_proc_defs);
    gchar *proc_str = g_strdup_printf ("Process order: %s", proc_list); 
    gchar *rev_str = g_strdup_printf ("Revision: %s", SVNVERSION );
    gchar *date_str = g_strdup_printf ("Date: %s", SVNDATE);
    gchar *url_str = g_strdup_printf ("URL: %s", SVNURL);

    str = g_strjoin ("\n", proc_str, rev_str, date_str, url_str, NULL);

    g_free (url_str);
    g_free (date_str);
    g_free (rev_str);
    g_free (proc_str);
    g_free (proc_list);
  }

  return str;
}

gboolean
sedflux_is_2d (Sedflux_state* self)
{
  eh_require (self);

  return self->is_2d;
}

void
_sedflux_save_time_variables (Sedflux_state* state)
{
  eh_require (state);

  {
    int dimen[3];
    if (state->thickness)
      eh_free (state->thickness);
    state->thickness = sedflux_get_value (state, "Thickness", dimen);
  }
}

Sedflux_state*
sedflux_initialize (const gint argc, const gchar* argv[])
{
  Sedflux_state* state = NULL;

  eh_require (argc>1);
  eh_require (argv && argv[0]);

  {
    GError* error = NULL;

    /* Create a new state. */
    state = sedflux_new ();

    { /* Parse command line arguments */
      Sedflux_param_st* p = sedflux_parse_command_line (argc, argv, &error);

      eh_exit_on_error (error,
                        "Error parsing sedflux initialization arguments");

      sedflux_set_init_file (state, p->init_file);
      sedflux_set_input_dir (state, p->input_dir);
      sedflux_set_working_dir (state, p->working_dir);
      sedflux_set_description (state, p->run_desc);
      sedflux_set_dimension (state, p->mode_2d);

      /* Setup the signal handling */
      if (p->set_signals)
        sed_signal_set_action();


      eh_free( p );
    }

    eh_info ("Creating project directory...");
    { /* Create the project directory and check permissions */
      sedflux_set_project_dirs (state, &error);
      eh_exit_on_error (error, "Error setting up project directory");
    }

    eh_info ("Printing info file...");
    { /* Print the info file */
      gchar* command_str = eh_render_command_str (argc, argv);
      sedflux_print_info_file (sedflux_init_file (state),
                               sedflux_working_dir (state),
                               command_str,
                               sedflux_description (state));
    }

    eh_info ("Scanning init file...");
    { /* Scan the init file. */
      eh_info ("Creating sedflux cube...");
      state->p = sed_cube_new_from_file (sedflux_init_file (state),
                                         sedflux_input_dir (state),
                                         &error);
      eh_exit_on_error (error, "%s: Error reading initialization file",
                               sedflux_init_file (state));

      eh_info ("Creating sedflux epoch queue...");
      state->q = sed_epoch_queue_new_full (sedflux_init_file (state),
                                           sedflux_input_dir (state),
                                           my_proc_defs, my_proc_family,
                                           NULL, &error);
      eh_exit_on_error (error, "%s: Error reading epoch file",
                                sedflux_init_file (state));
    }

    _sedflux_save_time_variables (state);
  }
  eh_info ("Sedflux is set up.");
  fprintf (stderr, "Sedflux state is %d\n", state);
  fprintf (stderr, "Sedflux size is %d\n", sed_cube_size (state->p));

  return state;
}

#if 0
Sedflux_state*
sedflux_initialize (const gchar* file, const gchar* prefix, int dimen)
{
  Sedflux_state* state = NULL;

  { /* Create a new state. */
    state = eh_new (Sedflux_state,1);

    state->q = NULL;
    state->p = NULL;
    state->init_file = g_strdup (file);
    state->prefix = g_strdup (prefix);
  }

  switch (dimen)
  {
    case 2:
      sed_mode_set (SEDFLUX_MODE_2D);
      break;
    case 3:
      sed_mode_set (SEDFLUX_MODE_3D);
      break;
    default:
      eh_require_not_reached ();
  }

  eh_require (sed_mode_is_2d() || sed_mode_is_3d());

  { /* Set up the sedflux working directory */
    GError* error = NULL;
    _sedflux_set_up (state->init_file, state->prefix, dimen, &error);
    eh_exit_on_error (error, "%s: Error setting up sedflux", state->init_file);
  }

  { /* Scan the init file. */
    GError* error = NULL;

    state->p = sed_cube_new_from_file (state->init_file, state->prefix,
                                       &error);
    eh_exit_on_error( error , "%s: Error reading initialization file" ,
                              state->init_file );

    state->q = sed_epoch_queue_new_full (state->init_file, state->prefix,
                                         my_proc_defs, my_proc_family,
                                         NULL, &error);
    eh_exit_on_error( error , "%s: Error reading epoch file" ,
                               state->init_file );
  }

  return state;
}
#endif

void
sedflux_run_time_step (Sedflux_state* state)
{
  eh_require( state );
  eh_require( state->q );
  eh_require( state->p );

  sed_epoch_queue_tic( state->q , state->p );
  _sedflux_save_time_variables (state);

  return;
}

void
sedflux_run (Sedflux_state* state)
{
  eh_require( state );
  eh_require( state->q );
  eh_require( state->p );

  sed_epoch_queue_run (state->q, state->p);
  _sedflux_save_time_variables (state);

  return;
}

void
sedflux_run_until (Sedflux_state* state, double then)
{
  eh_require( state );
  eh_require( state->q );
  eh_require( state->p );

  sed_epoch_queue_run_until( state->q , state->p , then );
  _sedflux_save_time_variables (state);

  return;
}

gchar**
sedflux_get_exchange_items (Sedflux_state* state)
{
  gchar** vals = NULL;

  vals = sed_measurement_all_names ();

  return vals;
}

gchar*
sedflux_get_exchange_item_unit (Sedflux_state* state, const gchar* name)
{
  gchar* unit = NULL;

  unit = sed_measurement_unit (name);

  return unit;
}

gchar**
sedflux_get_exchange_cube_items (Sedflux_state* state)
{
  gchar** vals = NULL;

  vals = sed_property_all_names ();

  return vals;
}

double*
sedflux_get_value_data (Sedflux_state* state, const char* val_s, int dimen[3])
{
  double * value_data = NULL;

  if (g_str_has_prefix (val_s, "Voxel")) {
    const gchar* value_name = val_s+strlen ("Voxel");
    value_data = sedflux_get_value_cube (state, value_name, dimen);
  }
  else if (g_ascii_strcasecmp (val_s, "Erosion")==0) {
    /* This is a time derivative */
    int i;
    const int len = sed_cube_size (state->p);
    double *erosion = eh_new (double, len);
    double* this_data;

    eh_require (state->thickness);

    this_data = sedflux_get_value (state, "Thickness", dimen);
    for (i=0; i<len; i++)
      //if (is_land_cell_id (state->p, i))
      if (sed_cube_elevation (state->p, 0, i)>.1)
        erosion[i] = 0;
      else
        erosion[i] = this_data[i] - state->thickness[i];

    {
      double max = -1e32;
      double min = 1e32;
      for (i=0; i<len; i++)
      {
        if (this_data[i]>max)
          max = erosion[i];
        if (this_data[i]<min)
          min = erosion[i];
      }

      fprintf (stderr, "Sedflux: Max erosion is %f\n", max);
      fprintf (stderr, "Sedflux: Min erosion is %f\n", min);
      fflush (stderr);
    }

    eh_free (this_data);

    value_data = erosion;

    /* Erode/Deposit to this elevation
    return sedflux_get_value (state, "Elevation", dimen);
    */
  }
  else
    value_data = sedflux_get_value (state, val_s, dimen);

  return value_data;

#if 0
  if (g_str_has_prefix (val_s, "SeaFloor"))
  {
    const gchar* value_name = val_s+strlen ("SeaFloor");
    if (g_ascii_strcasecmp (value_name, "Erosion")==0)
    { /* This is a time derivative */

      int i;
      const int len = sed_cube_size (state->p);
      double *erosion = eh_new (double, len);
      double* this_data;

      eh_require (state->thickness);

      this_data = sedflux_get_value (state, "Thickness", dimen);
      for (i=0; i<len; i++)
        //if (is_land_cell_id (state->p, i))
        if (sed_cube_elevation (state->p, 0, i)>.1)
          erosion[i] = 0;
        else
          erosion[i] = this_data[i] - state->thickness[i];

      {
        double max = -1e32;
        double min = 1e32;
        for (i=0; i<len; i++)
        {
          if (this_data[i]>max)
            max = erosion[i];
          if (this_data[i]<min)
            min = erosion[i];
        }

        fprintf (stderr, "Sedflux: Max erosion is %f\n", max);
        fprintf (stderr, "Sedflux: Min erosion is %f\n", min);
        fflush (stderr);
      }

      eh_free (this_data);

      return erosion;

      /* Erode/Deposit to this elevation
      return sedflux_get_value (state, "Elevation", dimen);
      */
    }
    else
      return sedflux_get_value (state, val_s+strlen ("SeaFloor"), dimen);
  }
  else
    return sedflux_get_value_cube (state, val_s, dimen);
#endif
}

double*
sedflux_get_double (Sedflux_state* state, const char* val_s, int * n_dim,
    int **dimen)
{
  double * d_vals = NULL;

  *dimen = sedflux_get_value_shape (state, val_s, n_dim);

  if (g_str_has_prefix (val_s, "Voxel")) {
    const gchar* value_name = val_s+strlen ("Voxel");
    d_vals = sedflux_get_value_cube (state, value_name, *dimen);
  }
  else if (g_ascii_strcasecmp (val_s, "Erosion")==0) {
    /* This is a time derivative */
    int i;
    const int len = sed_cube_size (state->p);
    double *erosion = eh_new (double, len);
    double* this_data;

    eh_require (state->thickness);

    //this_data = sedflux_get_value (state, "Thickness", *dimen);
    this_data = sedflux_get_value (state, "Thickness", NULL);
    for (i=0; i<len; i++)
      //if (is_land_cell_id (state->p, i))
      if (sed_cube_elevation (state->p, 0, i)>.1)
        erosion[i] = 0;
      else
        erosion[i] = this_data[i] - state->thickness[i];

    {
      double max = -1e32;
      double min = 1e32;
      for (i=0; i<len; i++)
      {
        if (this_data[i]>max)
          max = erosion[i];
        if (this_data[i]<min)
          min = erosion[i];
      }

      fprintf (stderr, "Sedflux: Max erosion is %f\n", max);
      fprintf (stderr, "Sedflux: Min erosion is %f\n", min);
      fflush (stderr);
    }

    eh_free (this_data);

    d_vals = erosion;

    /* Erode/Deposit to this elevation
    return sedflux_get_value (state, "Elevation", *dimen);
    */
  }
  else
  {
    gboolean mask_land = FALSE;

    if (g_str_has_prefix (val_s, "SeaFloor"))
    {
      mask_land = TRUE;
      val_s += strlen ("SeaFloor");
    }

    //d_vals = sedflux_get_value (state, val_s, *dimen);
    d_vals = sedflux_get_value (state, val_s, NULL);

    if (mask_land)
    {
      const int len = sed_cube_size (state->p);
      int i;
      for (i=0; i<len; i++)
        if (sed_cube_elevation (state->p, 0, i)>.1)
          d_vals[i] = -9999.;
    }

  }

  return d_vals;
#if 0
  if (g_str_has_prefix (val_s, "SeaFloor"))
  {
    const gchar* value_name = val_s+strlen ("SeaFloor");
    if (g_ascii_strcasecmp (value_name, "EROSION")==0)
    { /* This is a time derivative */

      int i;
      const int len = sed_cube_size (state->p);
      double *erosion = eh_new (double, len);
      double* this_data;

      eh_require (state->thickness);

      this_data = sedflux_get_value (state, "Thickness", *dimen);
      for (i=0; i<len; i++)
        //if (is_land_cell_id (state->p, i))
        if (sed_cube_elevation (state->p, 0, i)>.1)
          erosion[i] = 0;
        else
          erosion[i] = this_data[i] - state->thickness[i];

      {
        double max = -1e32;
        double min = 1e32;
        for (i=0; i<len; i++)
        {
          if (this_data[i]>max)
            max = erosion[i];
          if (this_data[i]<min)
            min = erosion[i];
        }

        fprintf (stderr, "Sedflux: Max erosion is %f\n", max);
        fprintf (stderr, "Sedflux: Min erosion is %f\n", min);
        fflush (stderr);
      }

      eh_free (this_data);

      return erosion;

      /* Erode/Deposit to this elevation
      return sedflux_get_value (state, "Elevation", *dimen);
      */
    }
    else
      return sedflux_get_value (state, val_s+strlen ("SeaFloor"), *dimen);
  }
  else
    return sedflux_get_value_cube (state, val_s, *dimen);
#endif
}

double*
sedflux_get_value (Sedflux_state* state, const char* val_s, int dimen[3])
{
  double* data = NULL;

  eh_return_val_if_fail (state, NULL);
  eh_return_val_if_fail (val_s, NULL);

  {
    Sed_measurement m = sed_measurement_new (val_s);

    eh_require (m);

    { /* Get values at all grid cells */
      gint64 id;
      Eh_ind_2 sub;
      const int len = sed_cube_size (state->p);

      data = eh_new (double, sed_cube_size (state->p));
      for (id=0; id<len; id++)
      {
        sub = sed_cube_sub (state->p, id);
        data[id] = sed_measurement_make (m, state->p, sub.i, sub.j);
      }

      if (dimen)
      {
        dimen[0] = sed_cube_n_x (state->p);
        dimen[1] = sed_cube_n_y (state->p);
        dimen[2] = 1;

        /* Dimensions are given from slowest to fastest. */
        dimen[0] = 1;
        dimen[1] = sed_cube_n_x (state->p);
        dimen[2] = sed_cube_n_y (state->p);

        {
          int i;
          int n_dim = 0;
          int * shape = sedflux_get_value_shape (state, val_s, &n_dim);
          for (i=0; i<n_dim; i++)
            dimen[i] = shape[i];
          g_free (shape);
        }
      }
    }

    sed_measurement_destroy( m );
   }

   return data;
}

double *
sedflux_get_surface_value (Sedflux_state* state, const char* val_s, double *dest, gint mask)
{
  double* data = NULL;

  eh_return_val_if_fail (state, NULL);
  eh_return_val_if_fail (val_s, NULL);
  eh_return_val_if_fail (dest, NULL);

  {
    Sed_measurement m = sed_measurement_new (val_s);

    eh_require (m);

    { /* Get values at all grid cells */
      gint i;
      const int len = sed_cube_size (state->p);
      Eh_ind_2 sub;

      eh_require (state)
      eh_require (state->p)

      for (i=0; i<len; ++i) {
        sub = sed_cube_sub (state->p, i);
        dest[i] = sed_measurement_make (m, state->p, sub.i, sub.j);
      }
      fprintf (stderr, "These are my elevations (all %d=%dx%d of them):\n", len, sedflux_get_nx (state), sedflux_get_ny (state));
      for (i=0; i<len; ++i)
        fprintf (stderr, "%d, %f\n", i, sed_cube_elevation (state->p, 0, i));
    }

    sed_measurement_destroy (m);

    if (mask) {
      const int len = sed_cube_size (state->p);
      int i;

      if (mask & MASK_LAND) {
        for (i=0; i<len; i++)
          if (sed_cube_elevation (state->p, 0, i) > .1)
            dest[i] = -9999.;
      }

      if (mask & MASK_OCEAN) {
        for (i=0; i<len; i++)
          if (sed_cube_elevation (state->p, 0, i) < -.1)
            dest[i] = -9999.;
      }
    }
  }

  return dest;
}

double *
sedflux_get_sediment_value (Sedflux_state* state, const char* val_s, double *dest)
{
  int dimen[3];
  double *buffer = NULL;

  buffer = sedflux_get_value_cube (state, val_s, dimen);

  if (!buffer)
    return NULL;

  {
    int i;
    const int len = dimen[0] * dimen[1] * dimen[2];
    fprintf (stderr, "These are my dimensions: %d x %d x %d\n", dimen[0], dimen[1], dimen[2]);
    for (i=0; i<len; ++i)
      dest[i] = buffer[i];
  }

  eh_free (buffer);

  return dest;
}

double*
sedflux_get_value_cube (Sedflux_state* state, const char* val_s, int dimen[3])
{
  double* data = NULL;

  eh_return_val_if_fail (state, NULL);
  eh_return_val_if_fail (val_s, NULL);

  {
    Sed_property p = sed_property_new (val_s);

    eh_require (p);

    { /* Get values at all grid cells */
      Eh_ndgrid g;

      g = sed_cube_property_subgrid (state->p, p, NULL, NULL, NULL);
      data = eh_ndgrid_start (g);

      /* Dimensions are given from slowest to fastest. */
      dimen[0] = eh_ndgrid_n (g,2);
      dimen[1] = eh_ndgrid_n (g,1);
      dimen[2] = eh_ndgrid_n (g,0);

      eh_ndgrid_destroy (g,FALSE);
    }

    sed_property_destroy (p);
   }

   return data;
}

void
sedflux_set_double (Sedflux_state* state, const char* val_s, double *vals,
    int n_dim, int * shape)
{
  if (g_ascii_strcasecmp (val_s, "Basement")==0)
    sedflux_set_basement (state, vals);
  else if (g_ascii_strcasecmp (val_s, "Uplift")==0)
    sedflux_set_uplift (state, vals);
  else if (g_ascii_strcasecmp (val_s, "DepositionToElevation")==0)
    sedflux_set_subaerial_deposition_to (state, vals);
  else if (g_ascii_strcasecmp (val_s, "WaterDischarge")==0)
    sedflux_set_discharge (state, vals);
  else if (g_ascii_strcasecmp (val_s, "BedLoadFlux")==0)
    sedflux_set_bed_load_flux (state, vals);
  else
  {
    fprintf (stderr,
             "sedflux_set_double: %s: Unknown value string\n", val_s);
    fflush (stderr);
  }
  return;
}

int*
sedflux_get_value_dimen (Sedflux_state* state, const char* val_s, int dimen[3])
{
  if (g_str_has_prefix (val_s, "Voxel")) {
    dimen[0] = -1;
    dimen[1] = sed_cube_n_y (state->p);
    dimen[2] = sed_cube_n_x (state->p);
  }
  else {
    dimen[0] = sed_cube_n_y (state->p);
    dimen[1] = sed_cube_n_x (state->p);
    dimen[2] = 1;
  }
#if 0
  if (g_str_has_prefix (val_s, "SeaFloor"))
  {
    dimen[0] = sed_cube_n_y (state->p);
    dimen[1] = sed_cube_n_x (state->p);
    dimen[2] = 1;
  }
  else
  {
    dimen[0] = -1;
    dimen[1] = sed_cube_n_y (state->p);
    dimen[2] = sed_cube_n_x (state->p);
  }
#endif
  return dimen;
}

int*
sedflux_get_value_shape (Sedflux_state* state, const char* val_s,
    int *n_dim)
{
  int * dimen = NULL;

  if (!g_str_has_prefix (val_s, "Voxel"))
  {
    if (sed_cube_n_x (state->p)==1)
      *n_dim = 1;
    else
      *n_dim = 2;

    dimen = g_new (int,*n_dim);
    /*
    dimen[0] = sed_cube_n_x (state->p);
    if (sed_cube_n_x (state->p)>1)
      dimen[1] = sed_cube_n_y (state->p);
    */

    /* Dimensions are given from slowest to fastest. */
    if (*n_dim == 1) {
      dimen[0] = sed_cube_n_y (state->p);
    } else {
      dimen[0] = sed_cube_n_x (state->p);
      dimen[1] = sed_cube_n_y (state->p);
    }
  }
  else
  {
    if (sed_cube_n_x (state->p)==1)
      *n_dim = 2;
    else
      *n_dim = 3;

    dimen = g_new (int,*n_dim);
    /*
    dimen[0] = -1;
    dimen[1] = sed_cube_n_y (state->p);
    if (sed_cube_n_x (state->p)>1)
      dimen[2] = sed_cube_n_x (state->p);
    */

    /* Dimensions are given from slowest to fastest. */
    if (*n_dim == 2) {
      dimen[0] = sed_cube_n_y (state->p);
      //dimen[1] = -1;
      dimen[1] = sed_cube_n_rows (state->p);
    } else {
      dimen[0] = sed_cube_n_x (state->p);
      dimen[1] = sed_cube_n_y (state->p);
      //dimen[2] = -1;
      dimen[2] = sed_cube_n_rows (state->p);
    }
  }

  return dimen;
}

int*
sedflux_get_grid_shape (Sedflux_state* state, const char* val_s,
    int *n_dim)
{
  int * shape = sedflux_get_value_shape (state, val_s, n_dim);
  const int len = *n_dim;
  /*
  int i = 0;
  for (i=0; i<len; i++)
    shape[i] += 1;
  */
  return shape;
}

double*
sedflux_get_value_res (Sedflux_state* state, const char* val_s, double res[3])
{
  if (!g_str_has_prefix (val_s, "Voxel"))
  {
    res[0] = sed_cube_x_res (state->p);
    res[1] = sed_cube_y_res (state->p);
    res[2] = sed_cube_z_res (state->p);
  }
  else
  {
    res[0] = sed_cube_z_res (state->p);
    res[1] = sed_cube_y_res (state->p);
    res[2] = sed_cube_x_res (state->p);
  }

  return res;
}

double*
sedflux_get_value_spacing (Sedflux_state* state, const char* val_s,
    int *n_dim)
{
  double * spacing = NULL;

  if (!g_str_has_prefix (val_s, "Voxel"))
  {
    if (sed_cube_n_x (state->p)==1)
      *n_dim = 1;
    else
      *n_dim = 2;

    spacing = g_new (double,*n_dim);
    /*
    spacing[0] = sed_cube_y_res (state->p);
    if (sed_cube_n_x (state->p)>1)
      spacing[1] = sed_cube_x_res (state->p);
    */

    if (*n_dim == 1) {
      spacing[0] = sed_cube_y_res (state->p);
    } else {
      spacing[0] = sed_cube_x_res (state->p);
      spacing[1] = sed_cube_y_res (state->p);
    }

  }
  else
  {
    if (sed_cube_n_x (state->p)==1)
      *n_dim = 2;
    else
      *n_dim = 3;

    spacing = g_new (double,*n_dim);
    /*
    spacing[0] = sed_cube_z_res (state->p);
    spacing[1] = sed_cube_y_res (state->p);
    if (sed_cube_n_x (state->p)>1)
      spacing[2] = sed_cube_x_res (state->p);
    */

    if (*n_dim == 2) {
      spacing[0] = sed_cube_y_res (state->p);
      spacing[1] = sed_cube_z_res (state->p);
    } else {
      spacing[0] = sed_cube_x_res (state->p);
      spacing[1] = sed_cube_y_res (state->p);
      spacing[2] = sed_cube_z_res (state->p);
    }
  }

  return spacing;
}

void
sedflux_set_uplift (Sedflux_state* state, const double* val)
{
  eh_require (state);
  eh_require (state->p);
  eh_require (val);

  {
    const gint len = sed_cube_size (state->p);
    gint i;

    for (i=0; i<len; i++) {
      if (val[i]>-999) {
        sed_column_adjust_base_height (sed_cube_col (state->p, i), val[i]);
      }
    }
  }

  return;
}
void
sedflux_set_basement (Sedflux_state* state, const double* val)
{
  eh_require (state);
  eh_require (state->p);
  eh_require (val);

  {
    const gint len = sed_cube_size (state->p);
    gint i;

    for (i=0; i<len; i++)
      sed_column_set_base_height (sed_cube_col (state->p, i), val[i]);
  }

  return;
}

void
sedflux_set_deposition (Sedflux_state* state, const double* val)
{
  eh_require (state);
  eh_require (state->p);
  eh_require (val);

  {
    Sed_cell add_cell = sed_cell_new_bedload (NULL, 1.);

    sed_cube_deposit_cell (state->p, val, add_cell);

    sed_cell_destroy (add_cell);
  }

  return;
}

void
sedflux_set_subaerial_deposition (Sedflux_state* state, const double* val)
{
  eh_require (state);
  eh_require (state->p);
  eh_require (val);

  {
    const Sed_cube c = state->p;
    Sed_cell subaqueous_cell = sed_cell_new_env ();

    { /* Deposit/erode on land.  Add up all sediment added to ocean. */
      const gint len = sed_cube_size (state->p);
      gint id;

      //eh_require (state->thickness);
      //eh_watch_int (len);

      for (id=0; id<len; id++)
      {
        //eh_watch_int (id);
        //eh_watch_dbl (val[id]);
//        if (is_land_cell_id (c, id))
//        {
//
          sed_cube_col_deposit_equal_amounts (c, id, val[id]);

//        }
/*
        else if (val[id]>0)
        {
          sed_cell_add_equal_amounts (subaqueous_cell, val[id]);
          state->thickness[id] += val[id];
        }
*/
      }

      //eh_watch_int (id);
    }

    //eh_message ("Done depositing");

#if 0
    { /* Put sediment added to ocean into the rivers to be added later. */
      Sed_riv* all = sed_cube_all_leaves (c);

      eh_watch_ptr (all);

      if (all)
      {
        gint n_leaves = g_strv_length ((gchar**)all);
        double t = sed_cell_size (subaqueous_cell)/n_leaves;
        Sed_riv* r;
        Sed_hydro river_data;

        eh_watch_int (n_leaves);
        eh_watch_dbl (t);

        sed_cell_resize (subaqueous_cell, t);

        for (r=all; *r; r++)
        {
          river_data = sed_river_hydro (*r);
          sed_hydro_add_cell (river_data, subaqueous_cell);
        }
      }
      else
        eh_message ("There is no river\n");
    }

    sed_cell_destroy (subaqueous_cell);
#endif
  }

  return;
}

void
sedflux_set_subaerial_deposition_to (Sedflux_state* state, const double* val)
{
  eh_require (state);
  eh_require (state->p);
  eh_require (val);

  {
    const Sed_cube c = state->p;
    const gint len = sed_cube_size (c);
    double* dz = sedflux_get_value (state, "Elevation", NULL);
    int i;

    for (i=0; i<len; i++)
    {
      if (fabs (val[i]-dz[i]>1e-3))
      {
/*
        eh_watch_int (i);
        eh_watch_dbl (val[i]);
        eh_watch_dbl (dz[i]);
        eh_watch_dbl (val[i]-dz[i]);
*/
      }
      dz[i] = val[i] - dz[i];
    }

    sedflux_set_subaerial_deposition (state, dz);

    eh_free (dz);
  }

  return;
}

void
sedflux_set_discharge (Sedflux_state* state, const double* val)
{
  eh_require (state);
  eh_require (state->p);
  eh_require (val);

  {
    sed_cube_set_discharge (state->p, val);
  }

  return;
}


void
sedflux_set_channel_bedload(Sedflux_state* state, const double* val)
{
  sed_cube_set_external_river_bedload(state->p, *val);
}


void
sedflux_set_channel_suspended_load(Sedflux_state* state, const double* val)
{
  sed_cube_set_external_river_suspended_load(state->p, *val);
}


void
sedflux_set_channel_width(Sedflux_state* state, const double* val)
{
  sed_cube_set_external_river_width(state->p, *val);
}


void
sedflux_set_channel_depth(Sedflux_state* state, const double* val)
{
  sed_cube_set_external_river_depth(state->p, *val);
}


void
sedflux_set_channel_velocity(Sedflux_state* state, const double* val)
{
  sed_cube_set_external_river_velocity(state->p, *val);
}


void
sedflux_set_sea_level(Sedflux_state* state, const double* val)
{
  sed_cube_set_sea_level(state->p, *val);
}


typedef struct
{
   double val;
   int    ind;
}
Flux_sort_st;

gint
cmp_flux_sort_vals_rev (Flux_sort_st *a, Flux_sort_st *b)
{
  if (a->val < b->val)
    return 1;
  else if (a->val > b->val)
    return -1;
  else
    return 0;
}

double *
sorted_shore_cells (const Sed_cube c, const double *val, int *len, int **sorted_ids)
{
  double * sorted_vals = NULL;

  eh_require (c);
  eh_require (val);
  eh_require (len);
  eh_require (sorted_ids);

  eh_debug ("Sorting fluxes...\n");

  {
    int i;
    const gint cube_len = sed_cube_size (c);
    GSList* top = NULL;
    Flux_sort_st * data = NULL;
    int list_len = 0;

    for (i=0; i<cube_len; i++) {
      if (is_shore_cell_id (c, i)) {
        data = eh_new (Flux_sort_st, 1);
        data->val = val[i];
        data->ind = i;
        top = g_slist_prepend (top, data);
        list_len++;
      }
    }
    //eh_watch_int (list_len);
    //eh_require (list_len>0);

    if (list_len > 0) {
      sorted_vals = eh_new (double, list_len);
      *sorted_ids = eh_new (int, list_len);
      *len = list_len;

      top = g_slist_sort (top, (GCompareFunc)cmp_flux_sort_vals_rev);
      {
        GSList* curr = NULL;
        for (curr=top, i=0; curr; curr=g_slist_next (curr), i++) {
          sorted_vals[i] = ((Flux_sort_st*)(curr->data))->val;
          (*sorted_ids)[i] = ((Flux_sort_st*)(curr->data))->ind;
          eh_free (curr->data);
        }
        g_slist_free (top);
      }
    }
    else {
      *len = 0;
      *sorted_ids = NULL;
      sorted_vals = NULL;
    }
  }

  return sorted_vals;
}

void
sedflux_set_bed_load_flux (Sedflux_state* state, const double* val)
{
  eh_debug ("Setting BedLoadFlux...\n");

  eh_require (state);
  eh_require (state->p);
  eh_require (val);

//  {
//    sed_cube_set_bed_load_flux (state->p, val);
//  }
#if 1
  {
    const Sed_cube c = state->p;
    int i;
    int * sorted_ids = NULL;
    double *sorted_flux = NULL;
    int len = 0;
    Sed_cell add_cell = sed_cell_new_env ();

    sorted_flux = sorted_shore_cells (state->p, val, &len, &sorted_ids);
    //eh_require (sorted_flux);
    //eh_require (sorted_ids);

    if (!sorted_flux) {
      eh_debug ("No shoreline cells with flux.");
    } else {
      //eh_watch_dbl (sorted_flux[0]);
      //eh_watch_int (sorted_ids[0]);
    }

    if (len>0 && sorted_flux[0]>0.) {
      Sed_riv* trunks = sed_cube_all_trunks (c);
      const gint n_trunks = g_strv_length ((gchar**)trunks);
      const double flux_to_thickness = sed_cube_time_step_in_years (c) /
                                         (sed_cube_y_res (c)*sed_cube_x_res (c));
      double total_flux = 0.;
      double top_n_total = 0.;

      for (i=0, total_flux=0.; i<len; i++)
        total_flux += sorted_flux[i];
      for (i=0, top_n_total=0.; i<n_trunks; i++)
        top_n_total += sorted_flux[i];
      for (i=0; i<n_trunks; i++)
        sorted_flux[i] *= total_flux/top_n_total;
      eh_watch_dbl (total_flux);
    
      for (i=0; i<n_trunks; i++) {
        Sed_riv* leaves = sed_river_leaves (trunks[i]);
        Sed_riv* leaf;
        const gint n_leaves = g_strv_length ((gchar**)leaves);
        const double total_thickness = sorted_flux[i] * flux_to_thickness / n_leaves;
        const Eh_ind_2 sub = sed_cube_sub (c, sorted_ids[i]);

        sed_cell_clear (add_cell);
        sed_cell_add_equal_amounts (add_cell, total_thickness);
        eh_watch_dbl (sed_cell_size (add_cell));

        for (leaf=leaves; *leaf; leaf++) {
          sed_river_add_cell (*leaf, add_cell);
          sed_river_set_hinge (*leaf, sub.i, sub.j);
        }

        g_free (leaves);
      }

      g_free (trunks);
    }

    g_free (sorted_ids);
    g_free (sorted_flux);
    sed_cell_destroy (add_cell);
  }
#else

  {
    double total_flux = 0;
    const Sed_cube c = state->p;
    int max_id = -1;

    { /* Calculate flux for all coastal pixels */
      const gint len = sed_cube_size (c);
      int i;
      double max_flux = -1e32;
      int n = 0;

      for (i=0; i<len; i++)
      {
        if (is_shore_cell_id (c, i))
        {
          total_flux += val[i];
          if (val[i]>max_flux)
          {
            max_flux = val[i];
            max_id = i;
          }
          n++;
        }
        total_flux += val[i];
      }

//eh_message ("Warning: Dividing total_flux by 10.");
//      total_flux /= 10.;

      eh_watch_dbl (max_flux);
      eh_watch_int (max_id);
      eh_watch_int (n);
    }

    if (total_flux>0)
    { /* Put sediment added to ocean into the rivers to be added later. */
      Sed_riv* all = sed_cube_all_leaves (c);
      //double const bed_load_density = sed_type_rho_sat (sed_sediment_type (NULL, 0));
      Sed_cell add_cell = sed_cell_new_env ();
      double total_thickness = total_flux * sed_cube_time_step_in_years (c) /
                               (sed_cube_y_res (c)*sed_cube_x_res (c));
eh_watch_dbl (sed_cube_time_step_in_years (c));
eh_watch_dbl (total_thickness);
      sed_cell_add_equal_amounts (add_cell, total_thickness);

      sed_cell_clear (sed_cube_to_add (c));
      sed_cell_add (sed_cube_to_add (c), add_cell);
      sed_cell_destroy (add_cell);
eh_watch_dbl (sed_cell_size (sed_cube_to_add (c)));

      /*  NOTE: for now assume flux is in m^3/year.  Convert to
                kg/s
      */
/*
      total_flux *= (bed_load_density/S_SECONDS_PER_YEAR);
*/
      if (all)
      {
        const gint n_leaves = g_strv_length ((gchar**)all);
        //const double flux = total_flux/n_leaves;
        Sed_riv* r;
        //Sed_hydro river_data;
        Eh_ind_2 sub = sed_cube_sub (c, max_id);

        //eh_watch_int (n_leaves);
        //eh_watch_dbl (flux);

        for (r=all; *r; r++)
        {
          fprintf (stderr, "DEBUG: Adding sediment to leaf\n");
          sed_river_set_hinge (*r, sub.i, sub.j);
          //river_data = sed_river_hydro (*r);
          //sed_river_set_bedload (*r, flux);
        }
        fprintf (stderr, "DEBUG: Added sediment to leaves\n");
      }
      else
        eh_message ("There is no river\n");

    }
  }
#endif
  eh_debug ("Set BedLoadFlux...\n");

  return;
}

double
sedflux_get_start_time (Sedflux_state* state)
{
  eh_require (state);

  Sed_epoch e = sed_epoch_queue_first (state->q);
  return sed_epoch_start (e);
}

double
sedflux_get_end_time (Sedflux_state* state)
{
  eh_require (state);

  Sed_epoch e = sed_epoch_queue_last (state->q);
  return sed_epoch_end (e);
}

double
sedflux_get_current_time (Sedflux_state* state)
{
  eh_require (state);
  return sed_cube_age_in_years (state->p);
}

int
sedflux_get_nx (Sedflux_state* state)
{
  eh_require (state);
  return sed_cube_n_x (state->p);
}

int
sedflux_get_ny (Sedflux_state* state)
{
  eh_require (state);
  return sed_cube_n_y (state->p);
}

int
sedflux_get_nz (Sedflux_state* state)
{
  eh_require (state);
  return sed_cube_n_rows (state->p);
}

double
sedflux_get_xres (Sedflux_state* state)
{
  eh_require (state);
  return sed_cube_x_res (state->p);
}

double
sedflux_get_yres (Sedflux_state* state)
{
  eh_require (state);
  return sed_cube_y_res (state->p);
}

double
sedflux_get_zres (Sedflux_state* state)
{
  eh_require (state);
  return sed_cube_z_res (state->p);
}

void
sedflux_finalize (Sedflux_state* state)
{
  if (state)
  {
    sed_epoch_queue_destroy (state->q);

    sed_cube_destroy (state->p);

    sed_sediment_unset_env ();
  }

  return;
}

