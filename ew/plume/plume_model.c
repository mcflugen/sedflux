#include <glib.h>

#include "plumevars.h"
#include "plumeinput.h"
#include "plume_local.h"

#include <utils/utils.h>
#include <sed/sed_sedflux.h>

#include "plume_model.h"
#include "bmi.h"

#define YEARS_PER_SECOND (3.1709791983764586e-08)
#define DAYS_PER_SECOND (1.1574074074074073e-05)

#ifndef LINE_MAX
# define LINE_MAX (2048)
#endif

struct _PlumeModel {
  //int input_var_name_count;
  //int output_var_name_count;
  //int output_var_grid_name_count;
  //const char ** input_var_names;
  //const char ** output_var_names;
  //const char ** output_var_grid_names;

  Plume_param_st *param;

  //int n_events;
  //int n_grains;
  //double * event_times;
  Sed_hydro * flood_events;
  //int current_event;
  
  //Sed_hydro flood_event;

  Eh_dbl_grid * deposit;
  double velocity;
  double width;
  double depth;
  double bedload;
  double qs;

  int cached;
  int n_dim;
  int rotate;
  int verbose;

  double time_in_days;
};


static void
destroy_deposit_grids(PlumeModel *self)
{
    if (self && self->deposit) {
        Eh_dbl_grid* d = NULL;

        for (d=self->deposit; *d; d++)
            eh_grid_destroy (*d, TRUE);

        eh_free (self->deposit);
    }
}


PlumeModel *
from_input_file(const char *fname)
{
    PlumeModel * self = g_new (PlumeModel, 1);
    GError* error = NULL;
    FILE * fp = fopen (fname, "r");
    gchar plume_file[LINE_MAX];
    gchar hydro_file[LINE_MAX];

    if (fp) {
        char line[LINE_MAX];
        fgets (line, LINE_MAX, fp);
        sscanf (line, "%s %s", plume_file, hydro_file);
        fclose (fp);
    } else
        return NULL;

    if (!error) {
        self->param = plume_scan_parameter_file (plume_file, &error);
        if (error) {
            fprintf (stderr, "%s\n", error->message);
            return NULL;
        }
    }

    if (!error) {
        self->flood_events = sed_hydro_scan (hydro_file, &error);
        if (error)
            return NULL;
    }
    return self;
}


PlumeModel *
from_defaults(void)
{
    PlumeModel * self = g_new (PlumeModel, 1);
    GError* error = NULL;
    gchar *buffer = sed_hydro_default_text ();

    self->param = plume_scan_parameter_file (NULL, &error);
    self->flood_events = sed_hydro_scan_text (buffer, &error);

    g_free (buffer);

    return self;
}


double
plume_get_start_time(PlumeModel * self)
{
  return 0.;
  //return self->event_times[0];
}


double
plume_get_current_time(PlumeModel * self)
{
  return self->time_in_days;
}


double
plume_get_end_time(PlumeModel * self)
{
    return BMI_FAILURE;
}


double *
plume_get_deposition_rate_buffer(PlumeModel * self)
{
  return eh_dbl_grid_data_start(self->deposit[0]);
}


void
plume_get_grid_shape(PlumeModel * self, int shape[2])
{
    shape[0] = eh_grid_n_x (self->deposit[0]);
    shape[1] = eh_grid_n_y (self->deposit[0]);
}


double *
plume_get_velocity_ptr(PlumeModel * self)
{
  return &(self->velocity);
}


double *
plume_get_width_ptr(PlumeModel * self)
{
  return &(self->width);
}


double *
plume_get_depth_ptr(PlumeModel * self)
{
  return &(self->depth);
}


double *
plume_get_bedload_ptr(PlumeModel * self)
{
  return &(self->bedload);
}


double *
plume_get_qs_ptr(PlumeModel * self)
{
  return &(self->qs);
}


void
plume_set_velocity(PlumeModel *self, double val)
{
    self->velocity = val;
    self->cached = FALSE;
}


void
plume_set_width(PlumeModel *self, double val)
{
    self->width = val;
    self->cached = FALSE;
}


void
plume_set_depth(PlumeModel *self, double val)
{
    self->depth = val;
    self->cached = FALSE;
}


void
plume_set_bedload(PlumeModel *self, double val)
{
    self->bedload = val;
    self->cached = FALSE;
}


void
plume_set_qs(PlumeModel *self, double val)
{
    self->qs = val;
    self->cached = FALSE;
}


void
plume_get_grid_spacing(PlumeModel * self, double spacing[2])
{
    const double * x = eh_grid_x_start (self->deposit[0]);
    const double * y = eh_grid_y_start (self->deposit[0]);
    spacing[0] = x[1] - x[0];
    spacing[1] = y[1] - y[0];
}


void
plume_get_grid_origin(PlumeModel * self, double origin[2])
{
    origin[0] = eh_grid_x_start (self->deposit[0])[0];
    origin[1] = eh_grid_y_start (self->deposit[0])[0];
}


int
plume_update_until (PlumeModel *self, double time)
{
  int rtn = BMI_FAILURE;

  //eh_return_val_if_fail (self, BMI_FAILURE);

  //if (time < self->time_in_days)
  //  return BMI_FAILURE;

  {
    int n;
    //int event_index = self->n_events;
    //const int n_event_times = self->n_events + 1;

    /* If out of time span, return an error */
    //if (time >= self->event_times[n_event_times-1])
    //  return BMI_FAILURE;

    /* Find index to the flood that contains the requested time */
    //for (n=self->current_event; n<n_event_times; n++) {
    //  if (time < self->event_times[n]) {
    //    event_index = n-1;
    //    break;
    //  }
    //}

    /* If out of time span, return an error */
    //if (event_index < 0)
    //  return BMI_FAILURE;

    /* If it is a different flood than the current one, run the new one */
    //if (event_index > self->current_event)
    if (!self->cached) {
      int n_grains, len;
      //Sed_hydro event = self->flood_events[event_index];
      Sed_hydro event = self->flood_events[0];

      sed_hydro_set_velocity (event, self->velocity);
      sed_hydro_set_width (event, self->width);
      sed_hydro_set_depth (event, self->depth);
      sed_hydro_set_bedload (event, self->bedload);
      sed_hydro_set_nth_concentration (event, 0, self->qs);

      destroy_deposit_grids(self);

      self->deposit = plume_wrapper (event, self->param, &len, &n_grains);
      self->cached = TRUE;

      //self->current_event = event_index;

      //self->velocity = sed_hydro_velocity (event);
      //self->width = sed_hydro_width (event);
      //self->depth = sed_hydro_depth (event);
      //self->bedload = sed_hydro_bedload (event);
      //self->qs = sed_hydro_nth_concentration (event, 0);
    }

    self->time_in_days = time;

    rtn = BMI_SUCCESS;
  }

  return rtn;
}


int
plume_update (PlumeModel *self)
{
  return plume_update_until(self, self->time_in_days + 1);
#if 0
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

      destroy_deposit_grids(self);

      self->deposit = dep_grid;

      self->current_event++;
      self->time_in_days = self->event_times[self->current_event];

      rtn = BMI_SUCCESS;
    }
  }
  return rtn;
#endif
}


int
plume_initialize (const char * config_file, PlumeModel **handle)
{
  int rtn = BMI_FAILURE;

  if (!g_thread_get_initialized ()) {
    g_thread_init (NULL);
    eh_init_glib ();
    g_log_set_handler (NULL, G_LOG_LEVEL_MASK, &eh_logger, NULL);
  }

  if (handle) {
    PlumeModel *self = NULL;

    if (config_file)
      self = from_input_file(config_file);
    else
      self = from_defaults();

    if (self) {
        //int n_events;

        //Sed_hydro *r = NULL;
        self->param->n_dim  = 2;
        self->param->rotate = 0;

        //n_events = 0;
        //for (r=self->flood_events; *r; r++) {
        //  n_events ++;
        //}

        //if (n_events > 0)
        {
          //int n;

          //self->n_events = n_events;
          //self->event_times = g_new (double, n_events+1);
          ////self->event_times[0] = sed_hydro_duration_in_seconds (self->flood_events[0]);
          //self->event_times[0] = 0.;

          //for (n=1; n<=n_events; n++) {
          //  self->event_times[n] = self->event_times[n-1] +
          //    sed_hydro_duration_in_seconds (self->flood_events[n-1]) * DAYS_PER_SECOND;
          //}

          //self->n_grains = sed_hydro_size (self->flood_events[0]);
          //self->current_event = -1;
          //self->time_in_days = self->event_times[0] - 1;
          self->time_in_days = 0.;
          self->cached = FALSE;

          { /* Run the first event */
            int len, n_grains;
            self->deposit = NULL;
            //rtn = plume_update_until (self, self->event_times[0]);
            rtn = plume_update (self);
          }
        }
      } else
          return BMI_FAILURE;

      if (rtn == BMI_SUCCESS) {
        *handle = self;

        rtn = BMI_SUCCESS;
      }
  }
  else
      rtn = BMI_FAILURE;

  return rtn;
}


int
plume_finalize (PlumeModel * self)
{
  if (self) {
    sed_hydro_array_destroy (self->flood_events);

    destroy_deposit_grids(self);

    g_free (self);
  }

  return BMI_SUCCESS;
}
