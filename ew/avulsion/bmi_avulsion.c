#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "bmi.h"

/* Implement this: Add model-specific includes */
#include "avulsion_api.h"


static int get_grid_size(void *self, int id, int *size);


static int
get_component_name (void *self, char * name)
{
    strncpy (name, "avulsion", BMI_MAX_COMPONENT_NAME);
    return BMI_SUCCESS;
}


#define INPUT_VAR_NAME_COUNT (10)
static const char *input_var_names[INPUT_VAR_NAME_COUNT] = {
    "channel_outflow_end__location_model_y_component",
    "channel_inflow_end_bed_load_sediment__mass_flow_rate",
    "channel_outflow_end_water__discharge",
    "avulsion_model__sediment_bed_load_exponent",
    "avulsion_model__water_discharge_exponent",
    "channel_inflow_end_water__discharge",
    "channel_outflow_end__location_model_x_component",
    "avulsion_model__random_walk_variance_constant",
    "channel_outflow_end_bed_load_sediment__mass_flow_rate",
    "surface__elevation"
};


static int
get_input_var_name_count(void *self, int *count)
{
    *count = INPUT_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}


static int
get_input_var_names(void *self, char **names)
{
    int i;
    for (i=0; i<INPUT_VAR_NAME_COUNT; i++) {
        strncpy(names[i], input_var_names[i], BMI_MAX_VAR_NAME);
    }
    return BMI_SUCCESS;
}


#define OUTPUT_VAR_NAME_COUNT (11)
static const char *output_var_names[OUTPUT_VAR_NAME_COUNT] = {
    "channel_outflow_end__location_model_y_component",
    "surface_bed_load_sediment__mass_flow_rate",
    "channel_inflow_end_bed_load_sediment__mass_flow_rate",
    "channel_outflow_end_water__discharge",
    "avulsion_model__sediment_bed_load_exponent",
    "channel_inflow_end_to_channel_outflow_end__angle",
    "avulsion_model__water_discharge_exponent",
    "channel_inflow_end_water__discharge",
    "channel_outflow_end__location_model_x_component",
    "avulsion_model__random_walk_variance_constant",
    "channel_outflow_end_bed_load_sediment__mass_flow_rate"
};


static int
get_output_var_name_count(void *self, int *count)
{
    *count = OUTPUT_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}


static int
get_output_var_names(void *self, char **names)
{
    int i;
    for (i=0; i<OUTPUT_VAR_NAME_COUNT; i++) {
        strncpy(names[i], output_var_names[i], BMI_MAX_VAR_NAME);
    }
    return BMI_SUCCESS;
}


static int
get_start_time(void * self, double *time)
{
    *time = 0;
    return BMI_SUCCESS;
}


static int
get_end_time(void * self, double *time)
{ /* Implement this: Set end time */
    *time = avulsion_get_end_time ((AvulsionModel*)self);
    return BMI_FAILURE;
}


static int
get_current_time(void * self, double *time)
{ /* Implement this: Set current time */
    *time = avulsion_get_current_time ((AvulsionModel*)self);
    return BMI_FAILURE;
}


static int
get_time_step(void * self, double *dt)
{ /* Implement this: Set time step */
    *dt = ((AvulsionModel*)self)->time_step;
    return BMI_FAILURE;
}


static int
get_time_units(void * self, char *units)
{
    strncpy(units, "d", BMI_MAX_UNITS_NAME);
    return BMI_SUCCESS;
}


static int
initialize(const char * file, void **handle)
{ /* Implement this: Create and initialize a model handle */
    return BMI_AVULSION_Initialize(file, handle);
}


static int
update_frac(void * self, double f)
{ /* Implement this: Update for a fraction of a time step */
    double now;
    double dt;

    get_current_time(self, &now);
    get_time_step(self, &dt);

    _avulsion_run_until ((AvulsionModel*)self, now + dt * f);

    return BMI_SUCCESS;
}


static int
update(void * self)
{
    return update_frac(self, 1.);
}


static int
update_until(void * self, double then)
{
    double dt;
    double now;

    if (get_time_step(self, &dt) == BMI_FAILURE)
        return BMI_FAILURE;

    if (get_current_time(self, &now) == BMI_FAILURE)
        return BMI_FAILURE;

    {
        int n;
        const double n_steps = (then - now) / dt;
        for (n=0; n<(int)n_steps; n++) {
            if (update(self) == BMI_FAILURE)
                return BMI_FAILURE;
        }

        if (update_frac(self, n_steps - (int)n_steps) == BMI_FAILURE)
            return BMI_FAILURE;
    }

    return BMI_SUCCESS;
}


static int
finalize(void * self)
{ /* Implement this: Clean up */
    return BMI_AVULSION_Finalize (self);
}


static int
get_var_grid(void *self, const char *name, int *grid)
{
    if (strcmp(name, "channel_outflow_end__location_model_y_component") == 0) {
        *grid = 1;
    } else if (strcmp(name, "surface_bed_load_sediment__mass_flow_rate") == 0) {
        *grid = 2;
    } else if (strcmp(name, "channel_inflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        *grid = 0;
    } else if (strcmp(name, "channel_outflow_end_water__discharge") == 0) {
        *grid = 1;
    } else if (strcmp(name, "avulsion_model__sediment_bed_load_exponent") == 0) {
        *grid = 0;
    } else if (strcmp(name, "channel_inflow_end_to_channel_outflow_end__angle") == 0) {
        *grid = 1;
    } else if (strcmp(name, "avulsion_model__water_discharge_exponent") == 0) {
        *grid = 0;
    } else if (strcmp(name, "channel_inflow_end_water__discharge") == 0) {
        *grid = 0;
    } else if (strcmp(name, "channel_outflow_end__location_model_x_component") == 0) {
        *grid = 1;
    } else if (strcmp(name, "avulsion_model__random_walk_variance_constant") == 0) {
        *grid = 0;
    } else if (strcmp(name, "channel_outflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        *grid = 1;
    } else if (strcmp(name, "surface__elevation") == 0) {
        *grid = 2;
    } else {
        *grid = -1; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_type(void *self, const char *name, char *type)
{
    if (strcmp(name, "channel_outflow_end__location_model_y_component") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "surface_bed_load_sediment__mass_flow_rate") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_inflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_outflow_end_water__discharge") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "avulsion_model__sediment_bed_load_exponent") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_inflow_end_to_channel_outflow_end__angle") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "avulsion_model__water_discharge_exponent") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_inflow_end_water__discharge") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_outflow_end__location_model_x_component") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "avulsion_model__random_walk_variance_constant") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_outflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "surface__elevation") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else {
        type[0] = '\0'; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_units(void *self, const char *name, char *units)
{
    if (strcmp(name, "channel_outflow_end__location_model_y_component") == 0) {
        strncpy(units, "m", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "surface_bed_load_sediment__mass_flow_rate") == 0) {
        strncpy(units, "kg / s", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_inflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        strncpy(units, "kg / s", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_outflow_end_water__discharge") == 0) {
        strncpy(units, "m^3 / s", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "avulsion_model__sediment_bed_load_exponent") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_inflow_end_to_channel_outflow_end__angle") == 0) {
        strncpy(units, "radian", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "avulsion_model__water_discharge_exponent") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_inflow_end_water__discharge") == 0) {
        strncpy(units, "m^3 / s", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_outflow_end__location_model_x_component") == 0) {
        strncpy(units, "m", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "avulsion_model__random_walk_variance_constant") == 0) {
        strncpy(units, "radian", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_outflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        strncpy(units, "kg / s", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "surface__elevation") == 0) {
        strncpy(units, "m", BMI_MAX_UNITS_NAME);
    } else {
        units[0] = '\0'; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_itemsize(void *self, const char *name, int *itemsize)
{
    if (strcmp(name, "channel_outflow_end__location_model_y_component") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "surface_bed_load_sediment__mass_flow_rate") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "channel_inflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "channel_outflow_end_water__discharge") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "avulsion_model__sediment_bed_load_exponent") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "channel_inflow_end_to_channel_outflow_end__angle") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "avulsion_model__water_discharge_exponent") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "channel_inflow_end_water__discharge") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "channel_outflow_end__location_model_x_component") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "avulsion_model__random_walk_variance_constant") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "channel_outflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "surface__elevation") == 0) {
        *itemsize = sizeof(double);
    } else {
        *itemsize = 0; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_nbytes(void *self, const char *name, int *nbytes)
{
    int id, size, itemsize;

    if (get_var_grid(self, name, &id) == BMI_FAILURE)
        return BMI_FAILURE;

    if (get_grid_size(self, id, &size) == BMI_FAILURE)
        return BMI_FAILURE;

    if (get_var_itemsize(self, name, &itemsize) == BMI_FAILURE)
        return BMI_FAILURE;

    *nbytes = itemsize * size;

    return BMI_SUCCESS;
}


static int
get_var_location(void *self, const char *name, char *loc)
{
    strncpy(loc, "node", BMI_MAX_VAR_NAME);
    return BMI_SUCCESS;
}


static int
get_grid_shape(void *self, int id, int *shape)
{
    if (id == 0) {
        shape[0] = 1;
    } else if (id == 1) {
        shape[0] = 1;
    } else if (id == 2) {
        shape[0] = avulsion_get_nx(self);
        shape[1] = avulsion_get_ny(self);
    } else {
        shape[0] = -1; shape[1] = -1;; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_grid_type(void *self, int id, char *type)
{
    if (id == 0) {
        strncpy(type, "scalar", 2048);
    } else if (id == 1) {
        strncpy(type, "vector", 2048);
    } else if (id == 2) {
        strncpy(type, "uniform_rectilinear", 2048);
    } else {
        type[0] = '\0'; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_grid_rank(void *self, int id, int *rank)
{
    if (id == 0) {
        *rank = 0;
    } else if (id == 1) {
        *rank = 1;
    } else if (id == 2) {
        *rank = 2;
    } else {
        *rank = -1; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_grid_size(void *self, int id, int *size)
{
    int rank;
    if (get_grid_rank(self, id, &rank) == BMI_FAILURE)
        return BMI_FAILURE;

    {
        int * shape = (int*) malloc(sizeof(int) * rank);
        int i;

        if (get_grid_shape(self, id, shape) == BMI_FAILURE)
          return BMI_FAILURE;

        *size = 1;
        for (i=0; i<rank; i++)
            *size *= shape[i];
        free(shape);
    }

    return BMI_SUCCESS;
}


static int
get_grid_spacing(void *self, int id, double *spacing)
{
    if (id == 0) {
        
    } else if (id == 1) {
        spacing[0] = -1.;
    } else if (id == 2) {
        spacing[0] = -1.; spacing[1] = -1.;
    } else {
        spacing[0] = -1.; spacing[1] = -1.;; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_grid_origin(void *self, int id, double *origin)
{
    if (id == 0) {
        
    } else if (id == 1) {
        origin[0] = -1.;
    } else if (id == 2) {
        origin[0] = -1.; origin[1] = -1.;
    } else {
        origin[0] = -1.; origin[1] = -1.;; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_value_ptr(void *self, const char *name, void **dest)
{
    AvulsionModel *model = (AvulsionModel*)self;

    if (strcmp(name, "channel_outflow_end__location_model_y_component") == 0) {
        *dest = model->mouth_y; /* Implement this: Pointer to channel_outflow_end__location_model_y_component */
    } else if (strcmp(name, "surface_bed_load_sediment__mass_flow_rate") == 0) {
        *dest = model->discharge; /* Implement this: Pointer to surface_bed_load_sediment__mass_flow_rate */
    } else if (strcmp(name, "channel_inflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        *dest = &model->sed_flux; /* Implement this: Pointer to channel_inflow_end_bed_load_sediment__mass_flow_rate */
    } else if (strcmp(name, "channel_outflow_end_water__discharge") == 0) {
        *dest = model->q; /* Implement this: Pointer to channel_outflow_end_water__discharge */
    } else if (strcmp(name, "avulsion_model__sediment_bed_load_exponent") == 0) {
        *dest = &model->bed_load_exponent; /* Implement this: Pointer to avulsion_model__sediment_bed_load_exponent */
    } else if (strcmp(name, "channel_inflow_end_to_channel_outflow_end__angle") == 0) {
        *dest = model->mouth_angle; /* Implement this: Pointer to channel_inflow_end_to_channel_outflow_end__angle */
    } else if (strcmp(name, "avulsion_model__water_discharge_exponent") == 0) {
        *dest = &model->discharge_exponent; /* Implement this: Pointer to avulsion_model__water_discharge_exponent */
    } else if (strcmp(name, "channel_inflow_end_water__discharge") == 0) {
        *dest = &model->init_discharge; /* Implement this: Pointer to channel_inflow_end_water__discharge */
    } else if (strcmp(name, "channel_outflow_end__location_model_x_component") == 0) {
        *dest = model->mouth_x; /* Implement this: Pointer to channel_outflow_end__location_model_x_component */
    } else if (strcmp(name, "avulsion_model__random_walk_variance_constant") == 0) {
        *dest = &model->variance; /* Implement this: Pointer to avulsion_model__random_walk_variance_constant */
    } else if (strcmp(name, "channel_outflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        *dest = model->mouth_qb; /* Implement this: Pointer to channel_outflow_end_bed_load_sediment__mass_flow_rate */
    } else if (strcmp(name, "surface__elevation") == 0) {
        *dest = model->elevation; /* Implement this: Pointer to surface__elevation */
    } else {
        *dest = NULL; return BMI_FAILURE;
    }

    if (*dest)
        return BMI_SUCCESS;
    else
        return BMI_FAILURE;
}


int
get_value(void * self, const char * name, void *dest)
{
    void *src = NULL;
    int nbytes = 0;

    if (get_value_ptr (self, name, &src) == BMI_FAILURE)
        return BMI_FAILURE;

    if (get_var_nbytes (self, name, &nbytes) == BMI_FAILURE)
        return BMI_FAILURE;

    memcpy(dest, src, nbytes);

    return BMI_SUCCESS;
}


static int
get_value_at_indices (void *self, const char *name, void *dest,
    int * inds, int len)
{
    void *src = NULL;
    int itemsize = 0;

    if (get_value_ptr(self, name, &src) == BMI_FAILURE)
        return BMI_FAILURE;

    if (get_var_itemsize(self, name, &itemsize) == BMI_FAILURE)
        return BMI_FAILURE;

    { /* Copy the data */
        int i;
        int offset;
        void * ptr;
        for (i=0, ptr=dest; i<len; i++, ptr+=itemsize) {
            offset = inds[i] * itemsize;
            memcpy (ptr, src + offset, itemsize);
        }
    }

    return BMI_SUCCESS;
}


static int
set_value (void *self, const char *name, void *array)
{
    void * dest = NULL;
    int nbytes = 0;

    if (get_value_ptr(self, name, &dest) == BMI_FAILURE)
        return BMI_FAILURE;

    if (get_var_nbytes(self, name, &nbytes) == BMI_FAILURE)
        return BMI_FAILURE;

    memcpy (dest, array, nbytes);

    return BMI_SUCCESS;
}


static int
set_value_at_indices (void *self, const char *name, int * inds, int len,
    void *src)
{
    void * to = NULL;
    int itemsize = 0;

    if (get_value_ptr (self, name, &to) == BMI_FAILURE)
        return BMI_FAILURE;

    if (get_var_itemsize(self, name, &itemsize) == BMI_FAILURE)
        return BMI_FAILURE;

    { /* Copy the data */
        int i;
        int offset;
        void * ptr;
        for (i=0, ptr=src; i<len; i++, ptr+=itemsize) {
            offset = inds[i] * itemsize;
            memcpy (to + offset, ptr, itemsize);
        }
    }
    return BMI_SUCCESS;
}


BMI_Model*
register_bmi_avulsion(BMI_Model *model)
{
    model->self = NULL;

    model->initialize = initialize;
    model->update = update;
    model->update_until = update_until;
    model->update_frac = update_frac;
    model->finalize = finalize;
    model->run_model = NULL;

    model->get_component_name = get_component_name;
    model->get_input_var_name_count = get_input_var_name_count;
    model->get_output_var_name_count = get_output_var_name_count;
    model->get_input_var_names = get_input_var_names;
    model->get_output_var_names = get_output_var_names;

    model->get_var_grid = get_var_grid;
    model->get_var_type = get_var_type;
    model->get_var_units = get_var_units;
    model->get_var_itemsize = get_var_itemsize;
    model->get_var_nbytes = get_var_nbytes;
    model->get_var_location = get_var_location;
    model->get_current_time = get_current_time;
    model->get_start_time = get_start_time;
    model->get_end_time = get_end_time;
    model->get_time_units = get_time_units;
    model->get_time_step = get_time_step;

    model->get_value = get_value;
    model->get_value_ptr = get_value_ptr;
    model->get_value_at_indices = get_value_at_indices;

    model->set_value = set_value;
    model->set_value_ptr = NULL;
    model->set_value_at_indices = set_value_at_indices;

    model->get_grid_rank = get_grid_rank;
    model->get_grid_size = get_grid_size;
    model->get_grid_type = get_grid_type;
    model->get_grid_shape = get_grid_shape;
    model->get_grid_spacing = get_grid_spacing;
    model->get_grid_origin = get_grid_origin;

    model->get_grid_x = NULL;
    model->get_grid_y = NULL;
    model->get_grid_z = NULL;

    return model;
}
