#include <stdio.h>
#include <string.h>
#include <glib.h>

//#include <utils/eh_messages.h>
#include "utils/utils.h"

#include "bmi_sedflux2d.h"
#include "sedflux_api.h"
#include <sed/sed_sedflux.h>

const double DAYS_PER_YEAR = 365.;
const double YEARS_PER_DAY = 0.0027397260273972603;

//typedef Sedflux_state _BMI_Model;
//typedef _BMI_Model Sedflux_state;
//struct _BMI_Model;
struct _BMI_Model {
  int input_var_name_count;
  int output_var_name_count;
  char **input_var_names;
  char **output_var_names;

  Sedflux_state *s;
};

void
set_input_var_names (BMI_Model *self, const char **input_var_names) {
  {
    char **names = g_strdupv ((char**)input_var_names);
    const int count = g_strv_length (names);

    self->input_var_name_count = count;
    self->input_var_names = names;
  }

  return;
}

void
set_output_var_names (BMI_Model *self, const char **output_var_names) {
  {
    char **names = g_strdupv ((char**)output_var_names);
    const int count = g_strv_length (names);

    self->output_var_name_count = count;
    self->output_var_names = names;
  }

  return;
}

int
has_input_var (const BMI_Model *self, const char *name) {
  char **names = self->input_var_names;
  const int count = self->input_var_name_count;
  int i;

  for (i=0; i<count; ++i)
    if (strcmp (names[i], name) == 0)
      return TRUE;
  return FALSE;
}

int
has_output_var (const BMI_Model *self, const char *name) {
  char **names = self->output_var_names;
  const int count = self->output_var_name_count;
  int i;

  for (i=0; i<count; ++i)
    if (strcmp (names[i], name) == 0)
      return TRUE;
  return FALSE;
}

int
BMI_SEDFLUX2D_Initialize (const char* file, BMI_Model **handle)
{
  int rtn = BMI_FAILURE;
  const char *input_var_names[] = {
    "bedrock_surface__elevation",
    "bedrock_surface__elevation_increment",
    "sediment__thickness_increment",
    "channel_outflow_end_water__discharge",
    "bed_load_sediment__mass_flow_rate",
    NULL,
  };
  const char *output_var_names[] = {
    "sea_water__depth",
    "sea_floor_surface__elevation",
    "sea_floor_surface__x_gradient_component",
    "sea_floor_surface__y_gradient_component",
    "sea_floor_sediment_grain__mean_diameter",
    "sea_floor_sediment__mean_duration_since_deposition",
    "sea_floor_sediment_sand__volume_fraction",
    "sea_floor_sediment_silt__volume_fraction",
    "sea_floor_sediment_clay__volume_fraction",
    "sea_floor_sediment_mud__volume_fraction",
    "sea_floor_sediment__bulk_density",
    "sea_floor_sediment__porosity",
    "sea_floor_sediment__permeability",
    "bedrock_surface__elevation",
    "sea_floor_sediment_model_grain_class_0__volume_fraction",

    "surface__elevation",
    "surface__x_gradient_component",
    "surface__y_gradient_component",
    "surface_sediment_grain__mean_diameter",
    "surface_sediment__mean_duration_since_deposition",
    "surface_sediment_sand__volume_fraction",
    "surface_sediment_silt__volume_fraction",
    "surface_sediment_clay__volume_fraction",
    "surface_sediment_mud__volume_fraction",
    "surface_sediment__bulk_density",
    "surface_sediment__porosity",
    "surface_sediment__permeability",
    "surface_sediment_model_grain_class_0__volume_fraction",

    "sediment_grain__mean_diameter",
    "sediment__mean_duration_since_deposition",
    "sediment__bulk_density",
    "sediment__porosity",
    "sediment__permeability",
    NULL,
  };

  eh_return_val_if_fail (handle, BMI_FAILURE_BAD_ARGUMENT_ERROR);

  if (!g_thread_get_initialized ()) {
    g_thread_init (NULL);
    eh_init_glib ();
    g_log_set_handler (NULL, G_LOG_LEVEL_MASK, &eh_logger, NULL);
  }

  if (handle) {
    Sedflux_state * self = NULL;
    char *args = NULL;

    if (file) { /* Read the config file */
      FILE * fp = fopen (file, "r");

      if (fp)
      {
        char line[2048];
        if (fgets (line, 2048, fp)!=line) {
          return BMI_FAILURE_SCAN_ERROR;
        }
        args = g_strdup (line);
      }
      else
        return BMI_FAILURE_BAD_FILE; // Unable to open file
    }
    else { /* Use a default value */
      args = g_strdup ("sedflux -3 -d /scratch/huttone/cca-projects/cem/0.1/internal/share/sedflux -i small_init.kvf --silent");
    }

    fprintf (stderr, "Initializing sedflux with these args: %s\n", args);
    if (args) { /* Initialize with these argments */
      int argc;
      char **argv;
      int i;

      argv = g_strsplit (args, " ", 0);
      argc = g_strv_length (argv);

      for (i=0; i<argc; i++)
        g_strstrip (argv[i]);

      self = sedflux_initialize (argc, (const char**)argv);

      if (self) {
        *handle = g_new (BMI_Model, 1);
        (*handle)->s = self;
        rtn = BMI_SUCCESS;
      }
      else
        return BMI_FAILURE_UNKNOWN_ERROR; // An unknown error occured

      g_strfreev (argv);

      set_input_var_names (*handle, input_var_names);
      set_output_var_names (*handle, output_var_names);
      
      rtn = BMI_SUCCESS;
    }
    else
      rtn = BMI_FAILURE_UNKNOWN_ERROR;
  }
  else
    rtn = BMI_FAILURE_BAD_ARGUMENT_ERROR; // Incorrect argument
  
  fprintf (stderr, "Sedflux is initialized !!!\n");
  return rtn;
}

int
BMI_SEDFLUX2D_Update (BMI_Model * state)
{
  sedflux_run_time_step (state->s);
  return BMI_SUCCESS;
}

int
BMI_SEDFLUX2D_Run_model (BMI_Model * state)
{
  sedflux_run (state->s);
  return BMI_SUCCESS;
}

int
BMI_SEDFLUX2D_Update_until (BMI_Model * state, double then)
{
  fprintf (stderr, "Running sedflux until: %f\n", then * YEARS_PER_DAY);
  sedflux_run_until (state->s, then * YEARS_PER_DAY);
  fprintf (stderr, "Ran sedflux until: %f\n", then * YEARS_PER_DAY);
  return BMI_SUCCESS;
}

int
BMI_SEDFLUX2D_Get_component_name (const BMI_Model * state, char *name)
{
  if (name) {
    strcpy (name, "Sedflux2D");
    return BMI_SUCCESS;
  }
  return BMI_FAILURE;
}

#define INPUT_VAR_NAME_COUNT (5)
/*
const char *_old_input_var_names[INPUT_VAR_NAME_COUNT] = {
  "Basement",
  "Uplift",
  "DepositionToElevation",
  "WaterDischarge",
  "BedLoadFlux"
};

const char *_input_var_names[INPUT_VAR_NAME_COUNT] = {
  "bedrock_surface__elevation",
  "bedrock_surface__elevation_increment",
  "sediment__thickness_increment",
  "channel_outflow_end_water__discharge",
  "bed_load_sediment__mass_flow_rate",
};
*/

int
BMI_SEDFLUX2D_Get_input_var_name_count (const BMI_Model * state, int *count)
{
  if (count) {
    *count = state->input_var_name_count;
    return BMI_SUCCESS;
  }
  return BMI_FAILURE;
}

int
BMI_SEDFLUX2D_Get_input_var_names (const BMI_Model * state, char **names)
{
  for (int i=0; i<state->input_var_name_count; i++)
    strncpy (names[i], state->input_var_names[i], BMI_SEDFLUX2D_VAR_NAME_MAX);
  return BMI_SUCCESS;
}
#define OUTPUT_VAR_NAME_COUNT (15)
/*
const char* _old_output_var_names[OUTPUT_VAR_NAME_COUNT] = {
  "SeaFloorSlope",
  "SeaFloorDepth",
  "SeaFloorElevation",
  "SeaFloorThickness",
  "SeaFloorGrain",
  "SeaFloorAge",
  "SeaFloorSand",
  "SeaFloorSilt",
  "SeaFloorClay",
  "SeaFloorMud",
  "SeaFloorFacies",
  "SeaFloorDensity",
  "SeaFloorPorosity",
  "SeaFloorPermeability",
  "SeaFloorBasement",
};

const char *_output_var_names[15] = {
  "sea_water__depth",
  "sea_floor_surface__elevation",
  "sea_floor_surface__x_gradient_component",
  "sea_floor_surface__y_gradient_component",
  "sea_floor_sediment_grain__mean_diameter",
  "sea_floor_sediment__mean_duration_since_deposition",
  "sea_floor_sediment_sand__volume_fraction",
  "sea_floor_sediment_silt__volume_fraction",
  "sea_floor_sediment_clay__volume_fraction",
  "sea_floor_sediment_mud__volume_fraction",
  "sea_floor_sediment__bulk_density",
  "sea_floor_sediment__porosity",
  "sea_floor_sediment__permeability",
  "bedrock_surface__elevation",
  "sea_floor_sediment_model_grain_class_0__volume_fraction",

  "surface__elevation",
  "surface__x_gradient_component",
  "surface__y_gradient_component",
  "surface_sediment_grain__mean_diameter",
  "surface_sediment__mean_duration_since_deposition",
  "surface_sediment_sand__volume_fraction",
  "surface_sediment_silt__volume_fraction",
  "surface_sediment_clay__volume_fraction",
  "surface_sediment_mud__volume_fraction",
  "surface_sediment__bulk_density",
  "surface_sediment__porosity",
  "surface_sediment__permeability",
  //"bedrock_surface__elevation",
  "surface_sediment_model_grain_class_0__volume_fraction",
};
*/
const char* _output_var_units[OUTPUT_VAR_NAME_COUNT] = {
   "meter/meter",
   "meter",
   "meter",
   "meter",
   "micrometer",
   "year",
   "1",
   "1",
   "1",
   "1",
   "1",
   "kilogram/meter^3",
   "1",
   "meter^2",
   "meter",
};

int
BMI_SEDFLUX2D_Get_output_var_name_count (const BMI_Model * state, int *count)
{
  if (count) {
    *count = state->output_var_name_count;
    return BMI_SUCCESS;
  }
  return BMI_FAILURE;
}

int
BMI_SEDFLUX2D_Get_output_var_names (const BMI_Model * state, char **names)
{
  for (int i=0; i<state->output_var_name_count; i++)
    strncpy (names[i], state->output_var_names[i], BMI_SEDFLUX2D_VAR_NAME_MAX);
  return BMI_SUCCESS;
}

int
BMI_SEDFLUX2D_Get_var_point_count (const BMI_Model * state, const char * name, int * count)
{
  int rtn = BMI_FAILURE;

  eh_return_val_if_fail (state && name && count, BMI_FAILURE);
  {
    int error;
    int rank;

    error = BMI_SEDFLUX2D_Get_var_rank (state, name, &rank);

    if (!error) {
      if (rank == 1)
        *count = sedflux_get_ny (state->s);
      else if (rank == 2)
        *count = sedflux_get_nz (state->s) * sedflux_get_ny (state->s);
      else
        error = BMI_FAILURE;
    }
    
    rtn = error;
  }

  return rtn;
}

int
BMI_SEDFLUX2D_Get_var_rank (const BMI_Model * state, const char * name, int * rank)
{
  int rtn = BMI_FAILURE;

  if (rank) {
    if (has_input_var (state, name) || has_output_var (state, name)) {
      if (g_str_has_prefix (name, "sediment_"))
        *rank = 2;
      else
        *rank = 1;
    }
    else
      return BMI_FAILURE;

    rtn = BMI_SUCCESS;
  }

  return rtn;
}

int
BMI_SEDFLUX2D_Get_grid_shape (const BMI_Model * state, const char * name, int * shape)
{
  int rtn = BMI_FAILURE;

  if (shape) {
    int error;
    int rank;

    error = BMI_SEDFLUX2D_Get_var_rank (state, name, &rank);

    if (!error) {
      if (rank == 1)
        shape[0] = sedflux_get_ny (state->s);
      else if (rank == 2) {
        shape[0] = sedflux_get_ny (state->s);
        shape[1] = sedflux_get_nz (state->s);
      }
      else
        error = BMI_FAILURE;
    }
    
    rtn = error;
  }

  return rtn;
}

int
BMI_SEDFLUX2D_Get_var_units (const BMI_Model * state, const char * name, char * unit_str)
{
  int rtn = BMI_FAILURE;

  if (unit_str) {
    char * units = sedflux_get_exchange_item_unit (state->s, name);

    if (units) {
      strncpy (unit_str, units, BMI_SEDFLUX2D_UNITS_NAME_MAX);
      g_free (units);
      rtn = BMI_SUCCESS;
    }
  }

  return BMI_SUCCESS;
}

int
BMI_SEDFLUX2D_Get_double (const BMI_Model *state, const char* val_s, double *dest)
{
  int rtn = BMI_FAILURE;

  if (dest) {
    {
      const char *name = NULL;

      if (has_output_var (state, val_s)) {
        if (strcmp (val_s, "sea_water__depth") == 0)
          name = "DEPTH";
        else if (g_str_has_suffix (val_s, "surface__elevation"))
          name = "ELEVATION";
        else if (g_str_has_suffix (val_s, "surface__x_gradient_component"))
          name = "XSLOPE";
        else if (g_str_has_suffix (val_s, "surface__y_gradient_component"))
          name = "YSLOPE";
        else if (g_str_has_suffix (val_s, "sediment_grain__mean_diameter"))
          name = "GRAIN";
        else if (g_str_has_suffix (val_s, "sediment__mean_duration_since_deposition"))
          name = "AGE";
        else if (g_str_has_suffix (val_s, "sediment_sand__volume_fraction"))
          name = "SAND";
        else if (g_str_has_suffix (val_s, "sediment_silt__volume_fraction"))
          name = "SILT";
        else if (g_str_has_suffix (val_s, "sediment_clay__volume_fraction"))
          name = "CLAY";
        else if (g_str_has_suffix (val_s, "sediment_mud__volume_fraction"))
          name = "MUD";
        else if (g_str_has_suffix (val_s, "sediment__bulk_density"))
          name = "DENSITY";
        else if (g_str_has_suffix (val_s, "sediment__porosity"))
          name = "POROSITY";
        else if (g_str_has_suffix (val_s, "sediment__permeability"))
          name = "PERMEABILITY";
        else if (strcmp (val_s, "bedrock_surface__elevation") == 0)
          name = "BASEMENT";
        else if (g_str_has_suffix (val_s, "sediment_model_grain_class_0__volume_fraction"))
          name = "";
      }
      else
        return BMI_FAILURE;

      if (name) {
        if (g_str_has_prefix (val_s, "sediment_")) {
          if (!sedflux_get_sediment_value (state->s, name, dest))
            return BMI_FAILURE;
        }
        else {
          gint mask = 0;

          if (g_str_has_prefix (val_s, "sea_floor_"))
            mask |= MASK_LAND;

          if (!sedflux_get_surface_value (state->s, name, dest, mask))
            return BMI_FAILURE;
          }
      }

      rtn = BMI_SUCCESS;
    }
  }

  return rtn;
}

int
BMI_SEDFLUX2D_Get_var_stride (const BMI_Model *state, const char* val_s, int *stride) {
  return -BMI_FAILURE;
}

int
BMI_SEDFLUX2D_Get_double_ptr (const BMI_Model *state, const char* val_s, double **dest)
{
  eh_return_val_if_fail (state && val_s && dest, BMI_FAILURE);
  return -BMI_FAILURE;
}

int
BMI_SEDFLUX2D_Set_double (BMI_Model *state, const char *val_s, const double *src)
{
  eh_return_val_if_fail (state && val_s && src, BMI_FAILURE);

  {
    if (strcmp (val_s, "bedrock_surface__elevation")==0)
      sedflux_set_basement (state->s, src);
    else if (strcmp (val_s, "bedrock_surface__elevation_increment")==0)
      sedflux_set_uplift (state->s, src);
    else if (strcmp (val_s, "sediment__thickness_increment")==0)
      sedflux_set_subaerial_deposition_to (state->s, src);
    else if (strcmp (val_s, "channel_outflow_end_water__discharge")==0)
      sedflux_set_discharge (state->s, src);
    else if (strcmp (val_s, "bed_load_sediment__mass_flow_rate")==0)
      sedflux_set_bed_load_flux (state->s, src);
    else
      return BMI_FAILURE;
  }
  return BMI_SUCCESS;
}

int
BMI_SEDFLUX2D_Get_grid_origin (const BMI_Model * state, const char* val_s,
    double *origin)
{
  int rtn = BMI_FAILURE;

  eh_return_val_if_fail (state && val_s && origin, BMI_FAILURE);

  {
    int rank;
    int error = BMI_SEDFLUX2D_Get_var_rank (state, val_s, &rank);

    if (!error) {
      if (rank == 1)
        origin[0] = 0.;
      else if (rank == 2) {
        origin[0] = 0.;
        origin[1] = 0.;
      }
      else
        return BMI_FAILURE;
      rtn = BMI_SUCCESS;
    }
  }

  return rtn;
}

int
BMI_SEDFLUX2D_Get_grid_spacing (const BMI_Model * state, const char* val_s, double *spacing)
{
  int rtn = BMI_FAILURE;

  eh_return_val_if_fail (state && val_s && spacing, BMI_FAILURE);

  {
    int rank;
    int error = BMI_SEDFLUX2D_Get_var_rank (state, val_s, &rank);

    if (!error) {
      if (rank == 1)
        spacing[0] = sedflux_get_yres (state->s);
      else if (rank == 2) {
        spacing[0] = sedflux_get_yres (state->s);
        spacing[1] = sedflux_get_zres (state->s);
      }
      else
        return BMI_FAILURE;
      rtn = BMI_SUCCESS;
    }
  }

  return rtn;
}

int
BMI_SEDFLUX2D_Get_start_time (const BMI_Model * state, double * time)
{
  int rtn = BMI_FAILURE;
  if (time) {
    *time = sedflux_get_start_time (state->s) * DAYS_PER_YEAR;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_SEDFLUX2D_Get_end_time (const BMI_Model * state, double * time)
{
  int rtn = BMI_FAILURE;
  if (time) {
    *time = sedflux_get_end_time (state->s) * DAYS_PER_YEAR;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_SEDFLUX2D_Get_current_time (const BMI_Model * state, double * time)
{
  int rtn = BMI_FAILURE;
  if (time) {
    *time = sedflux_get_current_time (state->s) * DAYS_PER_YEAR;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_SEDFLUX2D_Get_time_units (const BMI_Model * state, char * units)
{
  eh_return_val_if_fail (state && units, BMI_FAILURE);

  strcpy (units, "d");
  return BMI_SUCCESS;
}

int
BMI_SEDFLUX2D_Finalize (BMI_Model * state)
{
  fprintf (stderr, "Finalizing sedflux\n");
  sedflux_finalize (state->s);

  g_strfreev (state->output_var_names);
  g_strfreev (state->input_var_names);
  g_free (state);

  fprintf (stderr, "Finalized sedflux\n");
  return BMI_SUCCESS;
}

