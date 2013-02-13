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
  int input_var_name_count;
  int output_var_name_count;
  int output_var_grid_name_count;
  const char ** input_var_names;
  const char ** output_var_names;
  const char ** output_var_grid_names;

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
#define BMI_FAILURE_BAD_VAR_NAME_ERROR (5)

int
count_null_terminated_list (const char **names) {
  int count = 0;

  if (names) { /* Count number of names */
    const char **name;
    for (name=names, count=0; *name; ++name, ++count);
  }

  return count;
}

const char **
dup_null_terminated_list (const char **names, int *len) {
  const char **new_list = NULL;
  const int count = count_null_terminated_list (names);

  if (count > 0) {
    int i;

    new_list = (const char **) malloc (sizeof (char*) * (count + 1));
    for (i=0; i<count; i++) {
      new_list[i] = strdup (names[i]);
    }
    new_list[count] = NULL;
  }

  *len = count;

  return new_list;
}

void
free_null_terminated_list (char **names) {
  char **p;
  for (p=names; *p; ++p)
    free (*p);
  free (names);
}

const char **
concat_null_terminated_lists (const char **list1, const char **list2) {
  const char ** new_list = NULL;
  const int total_len = count_null_terminated_list (list1) +
    count_null_terminated_list (list2);

  if (total_len>0) {
    int i;
    const char **p;
    new_list = (const char **) malloc (sizeof (char*) * (total_len+1));

    for (i=0, p=list1; *p; ++i, ++p)
      new_list[i] = strdup (*p);
    for (p=list2; *p; ++i, ++p)
      new_list[i] = strdup (*p);

    new_list[total_len] = NULL;
  }

  return new_list;
}


void
set_input_var_names (BMI_Model *self) {
  const char *input_var_names[] = {
    "channel_outflow_end_water__speed",
    "channel_outflow_end__bankfull_width",
    "channel_outflow_end_water__depth",
    "channel_outflow_end_bed_load_sediment__mass_flow_rate",
    "channel_outflow_end_suspended_load_sediment__volume_concentration",
    "river_mouth_event__duration",
    NULL
  };

 
  {
    int count;
    const char **names = dup_null_terminated_list (input_var_names, &count);

    self->input_var_name_count = count;
    self->input_var_names = names;
  }

  return;
}

char**
deposition_rate_var_names (BMI_Model *self) {
  const int count = self->n_grains;
  char ** names = (char**) malloc (sizeof (char*) * (count+1));
  int i;

  for (i=0; i<count; ++i) {
    names[i] = (char*) malloc (sizeof (char*) * BMI_PLUME_VAR_NAME_MAX);
    g_snprintf (names[i], BMI_PLUME_VAR_NAME_MAX, "model_grain_class_%d__deposition_rate", i);
  }
  names[count] = NULL;

  return names;
}

void
set_output_var_names (BMI_Model *self) {
  const char *static_output_var_names[] = {
    "model_grain_class__count",
    NULL
  };
  const char **dynamic_output_var_names = (const char**)deposition_rate_var_names (self);
  const char **output_var_names = (const char**)concat_null_terminated_lists (
      dynamic_output_var_names, static_output_var_names);

  { /* Sets the names of variables that are grids (for now, same as the dynamic vars). */
    int count;
    const char **names = dup_null_terminated_list (dynamic_output_var_names, &count);

    self->output_var_grid_name_count = count;
    self->output_var_grid_names = names;
  }

  { /* Sets the names of all the output vars. */
    int count;
    const char **names = dup_null_terminated_list (output_var_names, &count);
    const char **p;

    self->output_var_name_count = count;
    self->output_var_names = names;
  }

  { /* Frees the NULL terminated list. */
    free_null_terminated_list ((char **)output_var_names);
    free_null_terminated_list ((char **)dynamic_output_var_names);
  }

  return;
}

int
has_input_var (BMI_Model *self, const char *name) {
  const char **names = self->input_var_names;
  const int count = self->input_var_name_count;
  int i;

  for (i=0; i<count; ++i)
    if (strcmp (names[i], name) == 0)
      return TRUE;
  return FALSE;
}

int
has_output_var (BMI_Model *self, const char *name) {
  const char **names = self->output_var_names;
  const int count = self->output_var_name_count;
  int i;

  for (i=0; i<count; ++i)
    if (strcmp (names[i], name) == 0)
      return TRUE;
  return FALSE;
}

int
has_output_var_grid (BMI_Model *self, const char *name) {
  const char **names = self->output_var_grid_names;
  const int count = self->output_var_grid_name_count;
  int i;

  for (i=0; i<count; ++i)
    if (strcmp (names[i], name) == 0)
      return TRUE;
  return FALSE;
}

int
BMI_PLUME_Get_var_rank (BMI_Model *self, const char * name, int *rank)
{
  if (rank) {
    if (strcmp (name, "channel_outflow_end_water__speed") == 0 ||
        strcmp (name, "channel_outflow_end__bankfull_width") == 0 ||
        strcmp (name, "channel_outflow_end_water__depth") == 0 ||
        strcmp (name, "channel_outflow_end_bed_load_sediment__mass_flow_rate") == 0 ||
        strcmp (name, "channel_outflow_end_suspended_load_sediment__volume_concentration") == 0 ||
        strcmp (name, "river_mouth_event__duration") == 0 ||
        strcmp (name, "model_grain_class__count") == 0)
      *rank = 0;
    else if (has_output_var_grid (self, name))
      *rank = 2;
    else
      return BMI_FAILURE_BAD_VAR_NAME_ERROR;
  }
  else
    return BMI_FAILURE;

  return BMI_SUCCESS;
}

int
BMI_PLUME_Get_var_stride (BMI_Model *self, const char *name, int *stride) {
  if (stride) {
    int rank;
    int error = BMI_PLUME_Get_var_rank (self, name, &rank);

    if (!error) {
      if (rank == 0)
        stride[0] = 1;
      else if (rank == 2) {
        stride[0] = eh_grid_n_y (self->deposit[0]);
        stride[1] = 1;
      }
    }
    else
      return error;
  }
  else
    return BMI_FAILURE;

  return BMI_SUCCESS;
}

int
BMI_PLUME_Initialize (const char * config_file, BMI_Model **handle)
{
  int rtn = BMI_FAILURE;

  if (!g_thread_get_initialized ()) {
    g_thread_init (NULL);
    eh_init_glib ();
    g_log_set_handler (NULL, G_LOG_LEVEL_MASK, &eh_logger, NULL);
  }

  if (handle) {
    BMI_Model * self = g_new (BMI_Model, 1);

    if (self) {
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
            rtn = BMI_PLUME_Update_until (self, self->event_times[0]);
          }
        }
      }

      if (!error && rtn == BMI_SUCCESS) {
        *handle = self;

        set_input_var_names (self);
        set_output_var_names (self);

        rtn = BMI_SUCCESS;
      }
    }
  }
  else
    rtn = BMI_FAILURE_BAD_ARGUMENT;
  return rtn;
}

int
BMI_PLUME_Update (BMI_Model *self)
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
BMI_PLUME_Update_until (BMI_Model *self, double time)
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
BMI_PLUME_Finalize (BMI_Model * self)
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
BMI_PLUME_Get_component_name (BMI_Model * self, char *name)
{
  int rtn = BMI_FAILURE;
  if (name) {
    strcpy (name, "Plume");
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_PLUME_Get_input_var_name_count (BMI_Model * self, int * input_var_name_count)
{
  int rtn = BMI_FAILURE;
  if (input_var_name_count) {
    *input_var_name_count = self->input_var_name_count;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_PLUME_Get_input_var_names (BMI_Model * self, char ** names)
{
  if (names && *names) {
    int i;
    for (i=0; i<self->input_var_name_count; i++) {
      strncpy (names[i], self->input_var_names[i], BMI_PLUME_VAR_NAME_MAX);
    }
  }
  return BMI_SUCCESS;
}

int
BMI_PLUME_Get_output_var_name_count (BMI_Model * self, int * output_var_name_count)
{
  int rtn = BMI_FAILURE;
  if (output_var_name_count) {
    *output_var_name_count = self->output_var_name_count;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_PLUME_Get_output_var_names (BMI_Model * self, char ** names)
{
  if (names && *names) {
    int i;
    for (i=0; i<self->output_var_name_count; i++) {
      strncpy (names[i], self->output_var_names[i], BMI_PLUME_VAR_NAME_MAX);
    }
  }
  return BMI_SUCCESS;
}

int
BMI_PLUME_Get_var_type (BMI_Model * self, const char * name, BMI_Var_type *type)
{
  int rtn = BMI_FAILURE;
  if (type) {
    *type = BMI_VAR_TYPE_DOUBLE;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_PLUME_Get_var_point_count (BMI_Model * self, const char * name, int *count)
{
  int rtn = BMI_FAILURE;
  if (count && name) {
    int rank;
    int error = BMI_PLUME_Get_var_rank (self, name, &rank);
    if (!error) {
      if (rank == 0)
        *count = 1;
      else if (rank == 2)
        *count = eh_grid_n_el (self->deposit[0]);
      else
        return BMI_FAILURE;
    }
    else
      return error;

    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_PLUME_Get_start_time (BMI_Model * self, double *time)
{
  int rtn = BMI_FAILURE;
  if (time) {
    *time = self->event_times[0];
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_PLUME_Get_current_time (BMI_Model * self, double *time)
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
BMI_PLUME_Get_end_time (BMI_Model * self, double *time)
{
  int rtn = BMI_FAILURE;
  if (time) {
    *time = self->event_times[self->n_events] - 1e-6;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_PLUME_Get_time_units (BMI_Model * self, char *units)
{
  eh_return_val_if_fail (self && units, BMI_FAILURE);

  strcpy (units, "d");
  return BMI_SUCCESS;
}

int
BMI_PLUME_Get_value (BMI_Model * self, const char *name, void *dest)
{
  int rtn = BMI_FAILURE;

  if (dest) {
    void *src = NULL;
    double scalar_value;
    int number_of_elements = 0;
    int err;

    err = BMI_PLUME_Get_var_point_count (self, name, &number_of_elements);
    if (err) {
      return err;
    }

    if (g_str_has_suffix (name, "__deposition_rate")) {
      gchar **strings = g_strsplit (name, "__", 2);
      gchar *object = strings[0];
      gint grain_number;

      sscanf (object, "model_grain_class_%d", &grain_number);

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

      if (strncmp (name, "channel_outflow_end_water__speed", BMI_PLUME_VAR_NAME_MAX) == 0) {
        *(double*)dest = sed_hydro_velocity (event);
      }
      else if (strncmp (name, "channel_outflow_end__bankfull_width", BMI_PLUME_VAR_NAME_MAX) == 0) {
        *(double*)dest = sed_hydro_width (event);
      }
      else if (strncmp (name, "channel_outflow_end_water__depth", BMI_PLUME_VAR_NAME_MAX) == 0) {
        *(double*)dest = sed_hydro_depth (event);
      }
      else if (strncmp (name, "channel_outflow_end_bed_load_sediment__mass_flow_rate", BMI_PLUME_VAR_NAME_MAX) == 0) {
        *(double*)dest = sed_hydro_bedload (event);
      }
      else if (strncmp (name, "model_grain_class__count", BMI_PLUME_VAR_NAME_MAX) == 0) {
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
BMI_PLUME_Get_double (BMI_Model * self, const char *name, double *dest)
{
  int rtn = BMI_FAILURE;

  if (dest) {
    double *src = NULL;
    BMI_Var_type type;

    BMI_PLUME_Get_var_type (self, name, &type);
    if (type == BMI_VAR_TYPE_DOUBLE)
      rtn = BMI_PLUME_Get_value (self, name, dest);
  }

  return rtn;
}

int
BMI_PLUME_Set_double (BMI_Model * self, const char *name, double *src)
{
  int rtn = BMI_FAILURE;

  if (src) {
    Sed_hydro event = self->flood_events[self->current_event];

    if (strncmp (name, "channel_outflow_end_water__speed", BMI_PLUME_VAR_NAME_MAX) == 0) {
      sed_hydro_set_velocity (event, *src);
    }
    else if (strncmp (name, "channel_outflow_end__bankfull_width", BMI_PLUME_VAR_NAME_MAX) == 0) {
      sed_hydro_set_width (event, *src);
    }
    else if (strncmp (name, "channel_outflow_end_water__depth", BMI_PLUME_VAR_NAME_MAX) == 0) {
      sed_hydro_set_depth (event, *src);
    }
    else if (strncmp (name, "channel_outflow_end_bed_load_sediment__mass_flow_rate", BMI_PLUME_VAR_NAME_MAX) == 0) {
      sed_hydro_set_bedload (event, *src);
    }
    else if (strncmp (name, "river_mouth_event__duration", BMI_PLUME_VAR_NAME_MAX) == 0) {
      sed_hydro_set_duration (event, *src);
    }
    else if (strncmp (name, "channel_outflow_end_suspended_load_sediment__volume_concentration", BMI_PLUME_VAR_NAME_MAX) == 0) {
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
BMI_PLUME_Get_double_ptr (BMI_Model * self, const char *name, double **dest)
{
  return -BMI_FAILURE;
}

int
BMI_PLUME_Get_grid_type (BMI_Model * self, const char * name, BMI_Grid_type *type)
{
  int rtn = BMI_FAILURE;
  if (type) {
    *type = BMI_GRID_TYPE_UNIFORM;
    rtn = BMI_SUCCESS;
  }
  return rtn;
}

int
BMI_PLUME_Get_grid_shape (BMI_Model * self, const char *name, int *shape)
{
  int rtn = BMI_FAILURE;

  if (shape) {
    int rank;
    int error = BMI_PLUME_Get_var_rank (self, name, &rank);
    if (!error) {
      if (rank == 0)
        shape[0] = 1;
      else if (rank == 2) {
        shape[0] = eh_grid_n_x (self->deposit[0]);
        shape[1] = eh_grid_n_y (self->deposit[0]);
      }
    }
    else
      return error;

    rtn = BMI_SUCCESS;
  }

  return rtn;
}

int
BMI_PLUME_Get_grid_spacing (BMI_Model * self, const char *name, double *spacing)
{
  int rtn = BMI_FAILURE;

  if (spacing) {
    int rank;
    int error = BMI_PLUME_Get_var_rank (self, name, &rank);
    if (!error) {
      if (rank == 0)
        spacing[0] = 0.;
      else if (rank == 2) {
        const double * x = eh_grid_x_start (self->deposit[0]);
        const double * y = eh_grid_y_start (self->deposit[0]);
        spacing[0] = x[1] - x[0];
        spacing[1] = y[1] - y[0];
      }
    }
    else
      return error;

    rtn = BMI_SUCCESS;
  }

  return rtn;
}

int
BMI_PLUME_Get_grid_origin (BMI_Model * self, const char *name, double *origin)
{
  int rtn = BMI_FAILURE;

  if (origin) {
    int rank;
    int error = BMI_PLUME_Get_var_rank (self, name, &rank);
    if (!error) {
      if (rank == 0)
        origin[0] = 0.;
      else if (rank == 2) {
        origin[0] = eh_grid_x_start (self->deposit[0])[0];
        origin[1] = eh_grid_y_start (self->deposit[0])[0];
      }
    }
    else
      return error;

    rtn = BMI_SUCCESS;
  }

  return rtn;
}

