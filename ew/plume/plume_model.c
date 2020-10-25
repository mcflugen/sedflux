#include <glib.h>

#include "plumevars.h"
#include "plumeinput.h"
#include "plume_local.h"

#include <utils/utils.h>
#include <sed/sed_sedflux.h>

#include "bmi.h"
#include "plume_model.h"

#define YEARS_PER_SECOND (3.1709791983764586e-08)
#define DAYS_PER_SECOND (1.1574074074074073e-05)

#ifndef LINE_MAX
    #define LINE_MAX (2048)
#endif

struct _PlumeModel {
    Plume_param_st* param;
    Sed_hydro flood_event;

    Eh_dbl_grid* deposit;
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
destroy_deposit_grids(PlumeModel* self)
{
    if (self && self->deposit) {
        Eh_dbl_grid* d = NULL;

        for (d = self->deposit; *d; d++) {
            eh_grid_destroy(*d, TRUE);
        }

        eh_free(self->deposit);
    }
}


PlumeModel*
from_input_file(PlumeModel* self, const char* fname)
{
    // PlumeModel * self = g_new (PlumeModel, 1);
    GError* error = NULL;
    FILE* fp = fopen(fname, "r");
    gchar plume_file[LINE_MAX];
    gchar hydro_file[LINE_MAX];

    if (self == NULL) {
        self = g_new(PlumeModel, 1);
    }

    if (fp) {
        char line[LINE_MAX];
        fgets(line, LINE_MAX, fp);
        sscanf(line, "%s %s", plume_file, hydro_file);
        fclose(fp);
    } else {
        return NULL;
    }

    if (!error) {
        self->param = plume_scan_parameter_file(plume_file, &error);

        if (error) {
            fprintf(stderr, "%s\n", error->message);
            return NULL;
        }
    }

    if (!error) {
        Sed_hydro* flood_events = sed_hydro_scan(hydro_file, &error);
        self->flood_event = flood_events[0];

        if (error) {
            return NULL;
        }

        self->velocity = sed_hydro_velocity(self->flood_event);
        self->width = sed_hydro_width(self->flood_event);
        self->depth = sed_hydro_depth(self->flood_event);
        self->bedload = sed_hydro_bedload(self->flood_event);
        self->qs = sed_hydro_nth_concentration(self->flood_event, 0);
    }

    return self;
}


PlumeModel*
from_defaults(PlumeModel* self)
{
    // PlumeModel * self = g_new (PlumeModel, 1);
    GError* error = NULL;
    gchar* buffer = sed_hydro_default_text();
    Sed_hydro* flood_events;

    if (self == NULL) {
        self = g_new(PlumeModel, 1);
    }

    self->param = plume_scan_parameter_file(NULL, &error);
    flood_events = sed_hydro_scan_text(buffer, &error);

    self->flood_event = flood_events[0];

    self->velocity = sed_hydro_velocity(flood_events[0]);
    self->width = sed_hydro_width(flood_events[0]);
    self->depth = sed_hydro_depth(flood_events[0]);
    self->bedload = sed_hydro_bedload(flood_events[0]);
    self->qs = sed_hydro_nth_concentration(flood_events[0], 0);

    g_free(buffer);

    return self;
}


double
plume_get_start_time(PlumeModel* self)
{
    return 0.;
    //return self->event_times[0];
}


double
plume_get_current_time(PlumeModel* self)
{
    return self->time_in_days;
}


double
plume_get_end_time(PlumeModel* self)
{
    return BMI_FAILURE;
}


double*
plume_get_deposition_rate_buffer(PlumeModel* self)
{
    return eh_dbl_grid_data_start(self->deposit[0]);
}


void
plume_get_grid_shape(PlumeModel* self, int shape[2])
{
    shape[0] = eh_grid_n_x(self->deposit[0]);
    shape[1] = eh_grid_n_y(self->deposit[0]);
}


double*
plume_get_velocity_ptr(PlumeModel* self)
{
    return &(self->velocity);
}


double*
plume_get_width_ptr(PlumeModel* self)
{
    return &(self->width);
}


double*
plume_get_depth_ptr(PlumeModel* self)
{
    return &(self->depth);
}


double*
plume_get_bedload_ptr(PlumeModel* self)
{
    return &(self->bedload);
}


double*
plume_get_qs_ptr(PlumeModel* self)
{
    return &(self->qs);
}


void
plume_set_velocity(PlumeModel* self, double val)
{
    self->velocity = val;
    self->cached = FALSE;
}


void
plume_set_width(PlumeModel* self, double val)
{
    self->width = val;
    self->cached = FALSE;
}


void
plume_set_depth(PlumeModel* self, double val)
{
    self->depth = val;
    self->cached = FALSE;
}


void
plume_set_bedload(PlumeModel* self, double val)
{
    self->bedload = val;
    self->cached = FALSE;
}


void
plume_set_qs(PlumeModel* self, double val)
{
    self->qs = val;
    self->cached = FALSE;
}


void
plume_get_grid_spacing(PlumeModel* self, double spacing[2])
{
    const double* x = eh_grid_x_start(self->deposit[0]);
    const double* y = eh_grid_y_start(self->deposit[0]);
    spacing[0] = x[1] - x[0];
    spacing[1] = y[1] - y[0];
}


void
plume_get_grid_origin(PlumeModel* self, double origin[2])
{
    origin[0] = eh_grid_x_start(self->deposit[0])[0];
    origin[1] = eh_grid_y_start(self->deposit[0])[0];
}


int
plume_update_until(PlumeModel* self, double time)
{
    int rtn = BMI_FAILURE;

    //eh_return_val_if_fail (self, BMI_FAILURE);

    //if (time < self->time_in_days)
    //  return BMI_FAILURE;

    {
        int n;

        /* If it is a different flood than the current one, run the new one */
        //if (event_index > self->current_event)
        if (!self->cached) {
            int n_grains, len;

            sed_hydro_set_velocity(self->flood_event, self->velocity);
            sed_hydro_set_width(self->flood_event, self->width);
            sed_hydro_set_depth(self->flood_event, self->depth);
            sed_hydro_set_bedload(self->flood_event, self->bedload);
            sed_hydro_set_nth_concentration(self->flood_event, 0, self->qs);

            destroy_deposit_grids(self);

            self->deposit = plume_wrapper(self->flood_event, self->param, &len, &n_grains);
            self->cached = TRUE;
        }

        self->time_in_days = time;

        rtn = BMI_SUCCESS;
    }

    return rtn;
}


int
plume_update(PlumeModel* self)
{
    return plume_update_until(self, self->time_in_days + 1);
}


PlumeModel*
plume_initialize(PlumeModel* self, const char* config_file)
{
    if (!g_thread_get_initialized()) {
        g_thread_init(NULL);
        eh_init_glib();
        g_log_set_handler(NULL, G_LOG_LEVEL_MASK, &eh_logger, NULL);
    }

    if (config_file == NULL || strcmp(config_file, "") == 0) {
        self = from_defaults(self);
    } else {
        self = from_input_file(self, config_file);
    }

    if (self) {
        self->param->n_dim  = 2;
        self->param->rotate = 0;

        {
            self->time_in_days = 0.;
            self->cached = FALSE;

            self->velocity = sed_hydro_velocity(self->flood_event);
            self->width = sed_hydro_width(self->flood_event);
            self->depth = sed_hydro_depth(self->flood_event);
            self->bedload = sed_hydro_bedload(self->flood_event);
            self->qs = sed_hydro_nth_concentration(self->flood_event, 0);

            { /* Run the first event */
                int len, n_grains;
                self->deposit = NULL;
                plume_update(self);
                self->time_in_days = plume_get_start_time(self);
            }
        }
    }

    return self;
}


int
plume_finalize(PlumeModel* self)
{
    if (self) {
        sed_hydro_destroy(self->flood_event);

        destroy_deposit_grids(self);

        g_free(self);
    }

    return BMI_SUCCESS;
}
