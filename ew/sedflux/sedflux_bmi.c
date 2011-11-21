#include <stdio.h>
#include <glib.h>

#include "bmi.h"
#include "sedflux_api.h"

void *
BMI_Initialize (const char* file)
{
  Sedflux_state * self = NULL;

  FILE * fp = fopen (file, "r");

  if (fp)
  {
    char args[2048];
    if (fgets (args, 2048, fp)==args)
    {
      int argc;
      char **argv;
      int i;

      argv = g_strsplit (args, " ", 0);
      argc = g_strv_length (argv);

      for (i=0; i<argc; i++)
        g_strstrip (argv[i]);

      self = sedflux_initialize (argc, (const char**)argv);

      g_strfreev (argv);
    }
  }

  return self;
}

void
BMI_Update (void * state)
{
  Sedflux_state * self = (Sedflux_state*) state;
  return sedflux_run_time_step (self);
}

void
BMI_Run_model (void * state)
{
  Sedflux_state * self = (Sedflux_state*) state;
  return sedflux_run (self);
}

void
BMI_Update_until (void * state, double then)
{
  Sedflux_state * self = (Sedflux_state*) state;
  return sedflux_run_until (self, then);
}

const char* _output_var_names[] = {
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
  "SeaFloorRiverMouth",
   NULL
};

const char* _output_var_units[] = {
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
   "meter",
   NULL
};

const char**
BMI_Get_output_var_names (void * state)
{
  return _output_var_names;
}

const char*
BMI_Get_var_units (void * state, const char * name)
{
  Sedflux_state * self = (Sedflux_state*) state;
  //return _output_var_units;
  //return sed_measurement_unit (name);
  return sedflux_get_exchange_item_unit (self, name);
}

double*
BMI_Get_double (void *state, const char* val_s, int * n_dim, int** shape)
{
  Sedflux_state * self = (Sedflux_state*) state;
  //*n_dim = 3;
  //*dimen = g_new (int, 3);
  //return sedflux_get_value_data (self, val_s, *dimen);
  return sedflux_get_double (self, val_s, n_dim, shape);
}

/* NOTE: use BMI_get_grid_shape instead */
int*
BMI_Get_grid_dimen (Sedflux_state* state, const char* val_s, int *n_dim)
{
  Sedflux_state * self = (Sedflux_state*) state;
  int * dimen = g_new (int, 3);
  *n_dim = 3;
  return sedflux_get_value_dimen (self, val_s, dimen);
}

/* NOTE: use BMI_get_grid_spacing instead */
double*
BMI_Get_grid_res (Sedflux_state* state, const char* val_s, int *n_dim)
{
  Sedflux_state * self = (Sedflux_state*) state;
  double * res = g_new (double, 3);
  *n_dim = 3;
  return sedflux_get_value_res (self, val_s, res);
}

int*
BMI_Get_grid_shape (void * state, const char* val_s, int *n_dim)
{
  Sedflux_state * self = (Sedflux_state*) state;
  return sedflux_get_value_shape (self, val_s, n_dim);
}

double*
BMI_Get_grid_lower_left_corner (void * state, const char* val_s,
    int *n_dim)
{
  Sedflux_state * self = (Sedflux_state*) state;
  double * ll = g_new0 (double, 3);
  *n_dim = 3;
  return ll;
}

double*
BMI_Get_grid_spacing (void * state, const char* val_s, int *n_dim)
{
  Sedflux_state * self = (Sedflux_state*) state;
  return sedflux_get_value_spacing (self, val_s, n_dim);
}

double
BMI_Get_start_time (void * state)
{
  Sedflux_state * self = (Sedflux_state*) state;
  return sedflux_get_start_time (self);
}

double
BMI_Get_end_time (void * state)
{
  Sedflux_state * self = (Sedflux_state*) state;
  return sedflux_get_end_time (self);
}

double
BMI_Get_current_time (void * state)
{
  Sedflux_state * self = (Sedflux_state*) state;
  return sedflux_get_current_time (self);
}

void
BMI_Finalize (void * state)
{
  Sedflux_state * self = (Sedflux_state*) state;
  return sedflux_finalize (self);
}

