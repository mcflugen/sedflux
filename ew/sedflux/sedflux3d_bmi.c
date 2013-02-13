#include <stdio.h>
#include <string.h>
#include <glib.h>

//#include <utils/eh_messages.h>
#include "utils/utils.h"

//#include "bmi_sedflux3d.h"
#include "bmi.h"
#include "bmi_sedflux3d.h"
#include "sedflux_api.h"

void set_input_var_names (BMI_Model *self, const char **input_var_names);
void set_output_var_names (BMI_Model *self, const char **input_var_names);
int has_input_var (const BMI_Model *self, const char *name);
int has_output_var (const BMI_Model *self, const char *name);

const double DAYS_PER_YEAR = 365.;
const double YEARS_PER_DAY = 0.0027397260273972603;

struct _BMI_Model {
  int input_var_name_count;
  int output_var_name_count;
  char **input_var_names;
  char **output_var_names;

  Sedflux_state *s;
};

int
BMI_SEDFLUX3D_Initialize (const char* file, BMI_Model **handle)
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
  
  return rtn;
}

int
BMI_SEDFLUX3D_Update (BMI_Model * state) {
  return BMI_SEDFLUX2D_Update (state);
}

int
BMI_SEDFLUX3D_Run_model (BMI_Model * state) {
  return BMI_SEDFLUX2D_Run_model (state);
}

int
BMI_SEDFLUX3D_Update_until (BMI_Model * state, double then) {
  return BMI_SEDFLUX2D_Update_until (state, then);
}

int
BMI_SEDFLUX3D_Get_component_name (const BMI_Model * state, char *name) {
  if (name) {
    strcpy (name, "Sedflux3D");
    return BMI_SUCCESS;
  }
  return BMI_FAILURE;
}

#define INPUT_VAR_NAME_COUNT (5)
/*
const char *_old_sedflux3d_input_var_names[INPUT_VAR_NAME_COUNT] = {
  "Basement",
  "Uplift",
  "DepositionToElevation",
  "WaterDischarge",
  "BedLoadFlux"
};

const char *_sedflux3d_input_var_names[INPUT_VAR_NAME_COUNT] = {
  "bedrock_surface__elevation",
  "bedrock_surface__elevation_increment",
  "sediment__thickness_increment",
  "channel_outflow_end_water__discharge",
  "bed_load_sediment__mass_flow_rate",
};
*/

int
BMI_SEDFLUX3D_Get_input_var_name_count (const BMI_Model * state, int *count) {
  return BMI_SEDFLUX2D_Get_input_var_name_count (state, count);
}

int
BMI_SEDFLUX3D_Get_input_var_names (const BMI_Model * state, char **names) {
  return BMI_SEDFLUX2D_Get_input_var_names (state, names);
}

#define OUTPUT_VAR_NAME_COUNT (15)
/*
const char* _old_sedflux3d_output_var_names[OUTPUT_VAR_NAME_COUNT] = {
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

const char *_sedflux3d_output_var_names[15] = {
  "sea_water__depth",
  "sea_floor_surface__elevation",
  "sea_floor_surface__x_gradient_component",
  "sea_floor_surface__y_gradient_component",
  "sea_floor_sediment_grain__mean_diameter",
  "sea_floor_sediment__mean_duration_since_deposition",
  "sand_in_sea_floor_sediment__volume_fraction",
  "silt_in_sea_floor_sediment__volume_fraction",
  "clay_in_sea_floor_sediment__volume_fraction",
  "mud_in_sea_floor_sediment__volume_fraction",
  "sea_floor_sediment__bulk_density",
  "sea_floor_sediment__porosity",
  "sea_floor_sediment__permeability",
  "bedrock_surface__elevation",
  "model_grain_class_0_in_sea_floor_sediment__volume_fraction",
};
*/
const char* _sedflux3d_output_var_units[OUTPUT_VAR_NAME_COUNT] = {
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
BMI_SEDFLUX3D_Get_output_var_name_count (const BMI_Model * state, int *count) {
  return BMI_SEDFLUX2D_Get_output_var_name_count (state, count);
}

int
BMI_SEDFLUX3D_Get_output_var_names (const BMI_Model * state, char **names) {
  return BMI_SEDFLUX2D_Get_output_var_names (state, names);
}

int
BMI_SEDFLUX3D_Get_var_point_count (const BMI_Model * state, const char * name, int * count)
{
  int rtn = BMI_FAILURE;

  eh_return_val_if_fail (state && name && count, BMI_FAILURE);
  {
    int error;
    int rank;

    error = BMI_SEDFLUX3D_Get_var_rank (state, name, &rank);

    if (!error) {
      if (rank == 2)
        *count = sedflux_get_ny (state->s) * sedflux_get_nx (state->s);
      else if (rank == 3)
        *count = sedflux_get_nz (state->s) * sedflux_get_ny (state->s) * sedflux_get_nx (state->s);
      else
        error = BMI_FAILURE;
    }
    
    rtn = error;
  }

  return rtn;
}

int
BMI_SEDFLUX3D_Get_var_rank (const BMI_Model * state, const char * name, int * rank)
{
  int rtn = BMI_FAILURE;

  if (rank) {
    if (has_input_var (state, name) || has_output_var (state, name)) {
      if (g_str_has_prefix (name, "sediment_"))
        *rank = 3;
      else
        *rank = 2;
    }
    else
      return BMI_FAILURE;

    rtn = BMI_SUCCESS;
  }

  return rtn;
}

int
BMI_SEDFLUX3D_Get_grid_shape (const BMI_Model * state, const char * name, int * shape)
{
  int rtn = BMI_FAILURE;

  if (shape) {
    int error;
    int rank;

    error = BMI_SEDFLUX3D_Get_var_rank (state, name, &rank);

    if (!error) {
      if (rank == 2) {
        shape[0] = sedflux_get_nx (state->s);
        shape[1] = sedflux_get_ny (state->s);
      }
      else if (rank == 3) {
        shape[0] = sedflux_get_nx (state->s);
        shape[1] = sedflux_get_ny (state->s);
        shape[2] = sedflux_get_nz (state->s);
      }
      else
        error = BMI_FAILURE;
    }
    
    rtn = error;
  }

  return rtn;
}

int
BMI_SEDFLUX3D_Get_grid_origin (const BMI_Model * state, const char* val_s,
    double *origin)
{
  int rtn = BMI_FAILURE;

  eh_return_val_if_fail (state && val_s && origin, BMI_FAILURE);

  {
    int rank;
    int error = BMI_SEDFLUX3D_Get_var_rank (state, val_s, &rank);

    if (!error) {
      if (rank == 2) {
        origin[0] = 0.;
        origin[1] = 0.;
      }
      else if (rank == 3) {
        origin[0] = 0.;
        origin[1] = 0.;
        origin[2] = 0.;
      }
      else
        return BMI_FAILURE;
      rtn = BMI_SUCCESS;
    }
  }

  return rtn;
}


int
BMI_SEDFLUX3D_Get_grid_spacing (const BMI_Model * state, const char* val_s, double *spacing)
{
  int rtn = BMI_FAILURE;

  eh_return_val_if_fail (state && val_s && spacing, BMI_FAILURE);

  {
    int rank;
    int error = BMI_SEDFLUX3D_Get_var_rank (state, val_s, &rank);

    if (!error) {
      if (rank == 2) {
        spacing[0] = sedflux_get_xres (state->s);
        spacing[1] = sedflux_get_yres (state->s);
      }
      else if (rank == 3) {
        spacing[0] = sedflux_get_xres (state->s);
        spacing[1] = sedflux_get_yres (state->s);
        spacing[2] = sedflux_get_zres (state->s);
      }
      else
        return BMI_FAILURE;
      rtn = BMI_SUCCESS;
    }
  }

  return rtn;
}

int
BMI_SEDFLUX3D_Get_var_units (const BMI_Model * state, const char * name, char * unit_str)
{
  int rtn = BMI_FAILURE;

  if (unit_str) {
    char * units = sedflux_get_exchange_item_unit (state->s, name);

    if (units) {
      strncpy (unit_str, units, BMI_SEDFLUX3D_UNITS_NAME_MAX);
      g_free (units);
      rtn = BMI_SUCCESS;
    }
  }

  return BMI_SUCCESS;
}

int
BMI_SEDFLUX3D_Get_double (const BMI_Model *state, const char* val_s, double *dest) {
  return BMI_SEDFLUX2D_Get_double (state, val_s, dest);
}

int
BMI_SEDFLUX3D_Get_var_stride (const BMI_Model *state, const char* val_s, int *stride) {
  return -BMI_FAILURE;
}

int
BMI_SEDFLUX3D_Get_double_ptr (const BMI_Model *state, const char* val_s, double **dest)
{
  eh_return_val_if_fail (state && val_s && dest, BMI_FAILURE);
  return -BMI_FAILURE;
}

int
BMI_SEDFLUX3D_Set_double (BMI_Model *state, const char *val_s, const double *src) {
  return BMI_SEDFLUX2D_Set_double (state, val_s, src);
}

int
BMI_SEDFLUX3D_Get_start_time (const BMI_Model * state, double * time) {
  return BMI_SEDFLUX2D_Get_start_time (state, time);
}

int
BMI_SEDFLUX3D_Get_end_time (const BMI_Model * state, double * time) {
  return BMI_SEDFLUX2D_Get_end_time (state, time);
}

int
BMI_SEDFLUX3D_Get_current_time (const BMI_Model * state, double * time) {
  return BMI_SEDFLUX2D_Get_current_time (state, time);
}

int
BMI_SEDFLUX3D_Get_time_units (const BMI_Model * state, char * units) {
  return BMI_SEDFLUX2D_Get_time_units (state, units);
}

int
BMI_SEDFLUX3D_Finalize (BMI_Model * state) {
  return BMI_SEDFLUX2D_Finalize (state);
}

