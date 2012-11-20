#include <glib.h>

#include "plumevars.h"
#include "plumeinput.h"
#include "plume_local.h"

#include <utils/utils.h>
#include <sed/sed_sedflux.h>

#include "plume_bmi.h"

#define YEARS_PER_SECOND (3.1709791983764586e-08)
#define DAYS_PER_SECOND (1.1574074074074073e-05)

#ifndef LINE_MAX
# define LINE_MAX (2048)
#endif

struct _BMI_Model {
  Plume_param_st *param;

  int n_events;
  int n_grains;
  double * event_times;
  Sed_hydro * flood_events;
  int current_event;

  Eh_dbl_grid * deposit;

  int n_dim;
  int rotate;
  int verbose;

  double time_in_days;
};

#define BMI_FAILURE_BAD_ARGUMENT (2)
#define BMI_FAILURE_BAD_FILE (3)
#define BMI_FAILURE_SCAN_ERROR_PARAM_FILE (4)
#define BMI_FAILURE_SCAN_ERROR_EVENT_FILE (5)

int
BMI_Initialize (const char * config_file, BMI_Model **handle)
{
  int rtn = BMI_FAILURE;

  if (!g_thread_get_initialized ()) {
    g_thread_init (NULL);
    eh_init_glib ();
    g_log_set_handler (NULL, G_LOG_LEVEL_MASK, &eh_logger, NULL);
  }

  if (handle) {
    BMI_Model * self = g_new (BMI_Model, 1);

    {
      GError* error = NULL;

      if (config_file) {
        FILE * fp = fopen (config_file, "r");
        gchar plume_file[LINE_MAX];
        gchar hydro_file[LINE_MAX];

        if (fp) {
          char line[LINE_MAX];
          fgets (line, LINE_MAX, fp);
          sscanf (line, "%s %s", plume_file, hydro_file);
          fclose (fp);
        }

        if (!error) {
          self->param = plume_scan_parameter_file (plume_file, &error);
          if (error) {
            fprintf (stderr, "%s\n", error->message);
            return BMI_FAILURE_SCAN_ERROR_PARAM_FILE;
          }
        }

        if (!error) {
          self->flood_events = sed_hydro_scan (hydro_file, &error);
          if (error)
            return BMI_FAILURE_SCAN_ERROR_EVENT_FILE;
        }
      }
      else {
        gchar *buffer = sed_hydro_default_text ();

        self->param = plume_scan_parameter_file (NULL, &error);
        self->flood_events = sed_hydro_scan_text (buffer, &error);

        g_free (buffer);
      }

      if (!error) {
        int n_events;

        Sed_hydro *r = NULL;
        self->param->n_dim  = 2;
        self->param->rotate = 0;

        n_events = 0;
        for (r=self->flood_events; *r; r++) {
          n_events ++;
        }

        if (n_events>0) {
          int n;

          self->n_events = n_events;
          self->event_times = g_new (double, n_events+1);
          //self->event_times[0] = sed_hydro_duration_in_seconds (self->flood_events[0]);
          self->event_times[0] = 0.;

          for (n=1; n<=n_events; n++) {
            self->event_times[n] = self->event_times[n-1] +
              sed_hydro_duration_in_seconds (self->flood_events[n-1]) * DAYS_PER_SECOND;
          }

          self->n_grains = sed_hydro_size (self->flood_events[0]);
          self->current_event = -1;
          self->time_in_days = self->event_times[0] - 1;

          { /* Run the first event */
            int len, n_grains;
            self->deposit = NULL;
            rtn = BMI_Update_until (self, self->event_times[0]);
          }
        }
      }

      if (!error && rtn == BMI_SUCCESS) {
        *handle = self;
        rtn = BMI_SUCCESS;
      }
    }
  }
  else
    rtn = BMI_FAILURE_BAD_ARGUMENT;
  return rtn;
}

int
BMI_Update (BMI_Model *self)
{
  int rtn = BMI_FAILURE;
  if (self) {

    if (self->current_event < self->n_events) {
      gint len;
      gint n_grains;
      Eh_dbl_grid *dep_grid = NULL;
      Sed_hydro event = self->flood_events[self->current_event];
      
      if (self->verbose)
        sed_hydro_fprint (stderr, event);

      dep_grid = plume_wrapper (event, self->param, &len, &n_grains);

      eh_message ("Finished.  Cleaning up.");
      if (self->deposit) {
        Eh_dbl_grid * d = NULL;

        for (d=self->deposit; *d; d++)
          eh_grid_destroy (*d, TRUE);

        eh_free (self->deposit);
      }

      self->deposit = dep_grid;

      self->current_event++;
      self->time_in_days = self->event_times[self->current_event];

      rtn = BMI_SUCCESS;
    }
  }
  return rtn;
}

int
BMI_Update_until (BMI_Model *self, double time)
{
  int rtn = BMI_FAILURE;

  eh_return_val_if_fail (self, BMI_FAILURE);

  if (time < self->time_in_days)
    return BMI_FAILURE;

  {
    int n;
    int event_index = self->n_events;
    const int n_event_times = self->n_events + 1;

    /* If out of time span, return an error */
    if (time >= self->event_times[n_event_times-1])
      return BMI_FAILURE;

    /* Find index to the flood that contains the requested time */
    for (n=self->current_event; n<n_event_times; n++) {
      if (time < self->event_times[n]) {
        event_index = n-1;
        break;
      }
    }

    /* If out of time span, return an error */
    if (event_index < 0)
      return BMI_FAILURE;

    /* If it is a different flood than the current one, run the new one */
    if (event_index > self->current_event) {
      int n_grains, len;
      Sed_hydro event = self->flood_events[event_index];

      if (self->deposit) {
        Eh_dbl_grid * d = NULL;

        for (d=self->deposit; *d; d++)
          eh_grid_destroy (*d, TRUE);

        eh_free (self->deposit);
      }

      self->deposit = plume_wrapper (event, self->param, &len, &n_grains);

      self->current_event = event_index;
    }

    self->time_in_days = time;

    rtn = BMI_SUCCESS;
  }

  return rtn;
}

int
BMI_Finalize (BMI_Model * self)
{
  if (self) {
    sed_hydro_array_destroy (self->flood_events);

    if (self->deposit) {
      Eh_dbl_grid* d = NULL;

      for (d=self->deposit; *d; d++)
        eh_grid_destroy (*d, TRUE);

     eh_free (self->deposit);
    }

    g_free (self);
  }
  return BMI_SUCCESS;
}

int
BMI_Get_component_name (BMI_Model * self, char *name)
{
  int rtn = BMI_FAILURE;
  if (name) {
    strcpy (name, "Plume");
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

#define BMI_INPUT_VAR_NAME_COUNT (5)
const char *input_var_names[BMI_INPUT_VAR_NAME_COUNT] = {
  "channel_outflow_end_water__speed",
  "channel_outflow_end__bankfull_width",
  "channel_outflow_end_water__depth",
  "channel_outflow_end_bed_load_sediment__mass_flow_rate",
  "channel_outflow_end_suspended_load_sediment__volume_concentration"
};

int
BMI_Get_input_var_name_count (BMI_Model * self, int * input_var_name_count)
{
  int rtn = BMI_FAILURE;
  if (input_var_name_count) {
    *input_var_name_count = BMI_INPUT_VAR_NAME_COUNT;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_Get_input_var_names (BMI_Model * self, char ** names)
{
  if (names && *names) {
    int i;
    for (i=0; i<BMI_INPUT_VAR_NAME_COUNT; i++) {
      strncpy (names[i], input_var_names[i], BMI_VAR_NAME_MAX);
    }
  }
  return BMI_SUCCESS;
}

const char *output_var_names[2] = {
  "grain_class_%d__deposition_rate",
  "grain_class__count"
};

int
BMI_Get_output_var_name_count (BMI_Model * self, int * output_var_name_count)
{
  int rtn = BMI_FAILURE;
  if (output_var_name_count) {
    *output_var_name_count = self->n_grains + 1;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_Get_output_var_names (BMI_Model * self, char ** names)
{
  if (names && *names) {
    int i;
    const int n_grains = self->n_grains;
    int n_output_vars;
    BMI_Get_output_var_name_count (self, &n_output_vars);

    for (i=0; i<n_grains; i++) {
      g_snprintf (names[i], BMI_VAR_NAME_MAX, output_var_names[0], i);
    }
    for (i=n_grains; i<n_output_vars; i++) {
      strncpy (names[i], output_var_names[i-n_grains+1], BMI_VAR_NAME_MAX);
    }
  }
  return BMI_SUCCESS;
}

int
BMI_Get_var_type (BMI_Model * self, const char * name, BMI_Var_type *type)
{
  int rtn = BMI_FAILURE;
  if (type) {
    *type = BMI_VAR_TYPE_DOUBLE;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_Get_var_rank (BMI_Model * self, const char * name, int *rank)
{
  int rtn = BMI_FAILURE;
  if (rank) {
    if (g_str_has_suffix (name, "__deposition_rate")) {
      *rank = 2;
    }
    else
      *rank = 0;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_Get_var_point_count (BMI_Model * self, const char * name, int *count)
{
  int rtn = BMI_FAILURE;
  if (count && name) {
    if (g_str_has_suffix (name, "__deposition_rate")) {
      *count = eh_grid_n_el (self->deposit[0]);
    }
    else
      *count = 1;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_Get_var_size (BMI_Model * self, const char * name, int *size)
{
  int rtn = BMI_FAILURE;
  if (size) {
    if (g_str_has_suffix (name, "__deposition_rate")) {
      *size = eh_grid_n_el (self->deposit[0]);
    }
    else
      *size = 1;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_Get_start_time (BMI_Model * self, double *time)
{
  int rtn = BMI_FAILURE;
  if (time) {
    *time = self->event_times[0];
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_Get_current_time (BMI_Model * self, double *time)
{
  int rtn = BMI_FAILURE;
  if (time) {
    fprintf (stderr, "Time is days is %f\n", self->time_in_days);
    *time = self->time_in_days;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_Get_end_time (BMI_Model * self, double *time)
{
  int rtn = BMI_FAILURE;
  if (time) {
    *time = self->event_times[self->n_events] - 1e-6;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_Get_time_units (BMI_Model * self, char *units)
{
  eh_return_val_if_fail (self && units, BMI_FAILURE);

  strcpy (units, "d");
  return BMI_SUCCESS;
}

int
BMI_Get_value (BMI_Model * self, const char *name, void *dest)
{
  int rtn = BMI_FAILURE;

  if (dest) {
    void *src = NULL;
    double scalar_value;
    int number_of_elements = 0;
    int err;

    err = BMI_Get_var_size (self, name, &number_of_elements);
    if (err) {
      return err;
    }

    if (g_str_has_suffix (name, "__deposition_rate")) {
      gchar **strings = g_strsplit (name, "__", 2);
      gchar *object = strings[0];
      gint grain_number;

      sscanf (object, "grain_class_%d", &grain_number);

      if (grain_number >= 0 && grain_number < self->n_grains)
        src = (void*)eh_dbl_grid_data_start (self->deposit[grain_number]);

      g_strfreev (strings);

      if (src) {
        memcpy (dest, src, sizeof (double) * number_of_elements);
        rtn = BMI_SUCCESS;
      }
    }
    else {
      Sed_hydro event = self->flood_events[self->current_event];

      if (strncmp (name, "channel_outflow_end_water__speed", BMI_VAR_NAME_MAX) == 0) {
        *(double*)dest = sed_hydro_velocity (event);
      }
      else if (strncmp (name, "channel_outflow_end__bankfull_width", BMI_VAR_NAME_MAX) == 0) {
        *(double*)dest = sed_hydro_width (event);
      }
      else if (strncmp (name, "channel_outflow_end_water__depth", BMI_VAR_NAME_MAX) == 0) {
        *(double*)dest = sed_hydro_depth (event);
      }
      else if (strncmp (name, "channel_outflow_end_bed_load_sediment__mass_flow_rate", BMI_VAR_NAME_MAX) == 0) {
        *(double*)dest = sed_hydro_bedload (event);
      }
      else if (strncmp (name, "grain_class__count", BMI_VAR_NAME_MAX) == 0) {
        *(int*)dest = self->n_grains;
      }
      else
        return BMI_FAILURE;

      rtn = BMI_SUCCESS;
    }
  }

  return rtn;
}

int
BMI_Get_double (BMI_Model * self, const char *name, double *dest)
{
  int rtn = BMI_FAILURE;

  if (dest) {
    double *src = NULL;
    BMI_Var_type type;

    BMI_Get_var_type (self, name, &type);
    if (type == BMI_VAR_TYPE_DOUBLE)
      rtn = BMI_Get_value (self, name, dest);
  }

  return rtn;
}

int
BMI_Set_double (BMI_Model * self, const char *name, double *src)
{
  int rtn = BMI_FAILURE;

  if (src) {
    Sed_hydro event = self->flood_events[self->current_event];

    if (strncmp (name, "channel_outflow_end_water__speed", BMI_VAR_NAME_MAX) == 0) {
      sed_hydro_set_velocity (event, *src);
    }
    else if (strncmp (name, "channel_outflow_end__bankfull_width", BMI_VAR_NAME_MAX) == 0) {
      sed_hydro_set_width (event, *src);
    }
    else if (strncmp (name, "channel_outflow_end_water__depth", BMI_VAR_NAME_MAX) == 0) {
      sed_hydro_set_depth (event, *src);
    }
    else if (strncmp (name, "channel_outflow_end_bed_load_sediment__mass_flow_rate", BMI_VAR_NAME_MAX) == 0) {
      sed_hydro_set_bedload (event, *src);
    }
    else if (strncmp (name, "river_mouth_event_duration", BMI_VAR_NAME_MAX) == 0) {
      sed_hydro_set_duration (event, *src);
    }
    else if (strncmp (name, "channel_outflow_end_suspended_load_sediment__volume_concentration", BMI_VAR_NAME_MAX) == 0) {
      double conc = *src / self->n_grains;
      int n;
      for (n=0; n<self->n_grains; n++)
        sed_hydro_set_nth_concentration (event, n, conc);
    }
    else
      return BMI_FAILURE;

    rtn = BMI_SUCCESS;
  }

  return rtn;
}

int
BMI_Get_double_ptr (BMI_Model * self, const char *name, double **dest)
{
  return -BMI_FAILURE;
}

int
BMI_Get_grid_type (BMI_Model * self, const char * name, BMI_Grid_type *type)
{
  int rtn = BMI_FAILURE;
  if (type) {
    *type = BMI_GRID_TYPE_UNIFORM;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_Get_grid_shape (BMI_Model * self, const char *name, int *shape)
{
  int rtn = BMI_FAILURE;

  if (shape) {
    if (g_str_has_suffix (name, "__deposition_rate")) {
      shape[0] = eh_grid_n_x (self->deposit[0]);
      shape[1] = eh_grid_n_y (self->deposit[0]);
    }

    rtn = BMI_SUCCESS;
  }

  return rtn;
}

int
BMI_Get_grid_spacing (BMI_Model * self, const char *name, double *spacing)
{
  int rtn = BMI_FAILURE;

  if (spacing) {
    if (g_str_has_suffix (name, "__deposition_rate")) {
      double * x = eh_grid_x_start (self->deposit[0]);
      double * y = eh_grid_y_start (self->deposit[0]);
      spacing[0] = x[1] - x[0];
      spacing[1] = y[1] - y[0];
    }

    rtn = BMI_SUCCESS;
  }

  return rtn;
}

int
BMI_Get_grid_origin (BMI_Model * self, const char *name, double *origin)
{
  int rtn = BMI_FAILURE;

  if (origin) {
    if (g_str_has_suffix (name, "__deposition_rate")) {
      origin[0] = eh_grid_x_start (self->deposit[0])[0];
      origin[1] = eh_grid_y_start (self->deposit[0])[0];
    }

    rtn = BMI_SUCCESS;
  }

  return rtn;
}

