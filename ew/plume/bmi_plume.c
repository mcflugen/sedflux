#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>


/* Implement this: Add model-specific includes */
#include "plume_model.h"
#include "bmi.h"

static int
get_grid_size(BMI_Model* self, int id, int* size);


static int
get_component_name(BMI_Model* self, char* name)
{
    strncpy(name, "plume", BMI_MAX_COMPONENT_NAME);
    return BMI_SUCCESS;
}


#define INPUT_VAR_NAME_COUNT (5)
static const char* input_var_names[INPUT_VAR_NAME_COUNT] = {
    "channel_outflow_end_water__speed",
    "channel_outflow_end_water__depth",
    "channel_outflow_end_suspended_load_sediment__volume_concentration",
    "channel_outflow_end_bed_load_sediment__mass_flow_rate",
    "channel_outflow_end__bankfull_width"
};


static int
get_input_item_count(BMI_Model* self, int* count)
{
    *count = INPUT_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}


static int
get_input_var_names(BMI_Model* self, char** names)
{
    int i;

    for (i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        strncpy(names[i], input_var_names[i], BMI_MAX_VAR_NAME);
    }

    return BMI_SUCCESS;
}


#define OUTPUT_VAR_NAME_COUNT (1)
static const char* output_var_names[OUTPUT_VAR_NAME_COUNT] = {
    "sea_bottom_sediment__deposition_rate"
};


static int
get_output_item_count(BMI_Model* self, int* count)
{
    *count = OUTPUT_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}


static int
get_output_var_names(BMI_Model* self, char** names)
{
    int i;

    for (i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
        strncpy(names[i], output_var_names[i], BMI_MAX_VAR_NAME);
    }

    return BMI_SUCCESS;
}


static int
get_start_time(BMI_Model* self, double* time)
{
    *time = plume_get_start_time((PlumeModel*)self->data);
    return BMI_SUCCESS;
}


static int
get_end_time(BMI_Model* self, double* time)
{
    /* Implement this: Set end time */
    *time = plume_get_end_time((PlumeModel*)self->data);
    return BMI_SUCCESS;
}


static int
get_current_time(BMI_Model* self, double* time)
{
    /* Implement this: Set current time */
    *time = plume_get_current_time((PlumeModel*)self->data);
    return BMI_SUCCESS;
}


static int
get_time_step(BMI_Model* self, double* dt)
{
    /* Implement this: Set time step */
    *dt = 1.;
    return BMI_SUCCESS;
}


static int
get_time_units(BMI_Model* self, char* units)
{
    strncpy(units, "d", BMI_MAX_UNITS_NAME);
    return BMI_SUCCESS;
}


static int
initialize(BMI_Model* self, const char* file)
{
    /* Implement this: Create and initialize a model handle */
    self->data = plume_initialize(NULL, file);
    return BMI_SUCCESS;
    // return plume_initialize(file, handle);
}


static int
update_frac(BMI_Model* self, double f)
{
    /* Implement this: Update for a fraction of a time step */
    double dt;

    if (self->get_time_step(self, &dt) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    return plume_update_until((PlumeModel*)self->data, f * dt);
}


static int
update(BMI_Model* self)
{
    return update_frac(self, 1.);
}


static int
update_until(BMI_Model* self, double then)
{
    return plume_update_until((PlumeModel*)self->data, then);
}


static int
finalize(BMI_Model* self)
{
    /* Implement this: Clean up */
    return plume_finalize((PlumeModel*)self->data);
}


static int
get_var_grid(BMI_Model* self, const char* name, int* grid)
{
    if (strcmp(name, "channel_outflow_end_water__speed") == 0) {
        *grid = 0;
    } else if (strcmp(name, "channel_outflow_end_water__depth") == 0) {
        *grid = 0;
    } else if (strcmp(name,
            "channel_outflow_end_suspended_load_sediment__volume_concentration") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_bottom_sediment__deposition_rate") == 0) {
        *grid = 1;
    } else if (strcmp(name, "channel_outflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        *grid = 0;
    } else if (strcmp(name, "channel_outflow_end__bankfull_width") == 0) {
        *grid = 0;
    } else {
        *grid = -1;
        return BMI_FAILURE;
    }

    return BMI_SUCCESS;
}


static int
get_var_type(BMI_Model* self, const char* name, char* type)
{
    if (strcmp(name, "channel_outflow_end_water__speed") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_outflow_end_water__depth") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name,
            "channel_outflow_end_suspended_load_sediment__volume_concentration") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment__deposition_rate") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_outflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_outflow_end__bankfull_width") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else {
        type[0] = '\0';
        return BMI_FAILURE;
    }

    return BMI_SUCCESS;
}


static int
get_var_units(BMI_Model* self, const char* name, char* units)
{
    if (strcmp(name, "channel_outflow_end_water__speed") == 0) {
        strncpy(units, "m / s", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_outflow_end_water__depth") == 0) {
        strncpy(units, "m", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name,
            "channel_outflow_end_suspended_load_sediment__volume_concentration") == 0) {
        strncpy(units, "1", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment__deposition_rate") == 0) {
        strncpy(units, "m / d", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_outflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        strncpy(units, "kg / s", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_outflow_end__bankfull_width") == 0) {
        strncpy(units, "m", BMI_MAX_UNITS_NAME);
    } else {
        units[0] = '\0';
        return BMI_FAILURE;
    }

    return BMI_SUCCESS;
}


static int
get_var_itemsize(BMI_Model* self, const char* name, int* itemsize)
{
    if (strcmp(name, "channel_outflow_end_water__speed") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "channel_outflow_end_water__depth") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name,
            "channel_outflow_end_suspended_load_sediment__volume_concentration") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_sediment__deposition_rate") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "channel_outflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "channel_outflow_end__bankfull_width") == 0) {
        *itemsize = sizeof(double);
    } else {
        *itemsize = 0;
        return BMI_FAILURE;
    }

    return BMI_SUCCESS;
}


static int
get_var_nbytes(BMI_Model* self, const char* name, int* nbytes)
{
    int id, size, itemsize;

    if (self->get_var_grid(self, name, &id) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    if (self->get_grid_size(self, id, &size) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    if (self->get_var_itemsize(self, name, &itemsize) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    *nbytes = itemsize * size;

    return BMI_SUCCESS;
}


static int
get_var_location(BMI_Model* self, const char* name, char* loc)
{
    strncpy(loc, "node", BMI_MAX_VAR_NAME);
    return BMI_SUCCESS;
}


static int
get_grid_shape(BMI_Model* self, int id, int* shape)
{
    if (id == 1) {
        plume_get_grid_shape((PlumeModel*)self->data, shape);
    } else {
        return BMI_FAILURE;
    }

    return BMI_SUCCESS;
}


static int
get_grid_type(BMI_Model* self, int id, char* type)
{
    if (id == 0) {
        strncpy(type, "none", 2048);
    } else if (id == 1) {
        strncpy(type, "uniform_rectilinear", 2048);
    } else {
        type[0] = '\0';
        return BMI_FAILURE;
    }

    return BMI_SUCCESS;
}


static int
get_grid_rank(BMI_Model* self, int id, int* rank)
{
    if (id == 0) {
        *rank = 0;
    } else if (id == 1) {
        *rank = 2;
    } else {
        *rank = -1;
        return BMI_FAILURE;
    }

    return BMI_SUCCESS;
}


static int
get_grid_size(BMI_Model* self, int id, int* size)
{
    int rank;

    if (self->get_grid_rank(self, id, &rank) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    if (rank == 0) {
        *size = 1;
    } else {
        int* shape = (int*) malloc(sizeof(int) * rank);
        int i;

        if (self->get_grid_shape(self, id, shape) == BMI_FAILURE) {
            return BMI_FAILURE;
        }

        *size = 1;

        for (i = 0; i < rank; i++) {
            *size *= shape[i];
        }

        free(shape);
    }

    return BMI_SUCCESS;
}


static int
get_grid_spacing(BMI_Model* self, int id, double* spacing)
{
    if (id == 1) {
        plume_get_grid_spacing((PlumeModel*)self->data, spacing);
    } else {
        return BMI_FAILURE;
    }

    return BMI_SUCCESS;
}


static int
get_grid_origin(BMI_Model* self, int id, double* origin)
{
    if (id == 1) {
        plume_get_grid_origin((PlumeModel*)self->data, origin);
    } else {
        return BMI_FAILURE;
    }

    return BMI_SUCCESS;
}


static int
get_value_ptr(BMI_Model* self, const char* name, void** dest)
{
    if (strcmp(name, "channel_outflow_end_water__speed") == 0) {
        *dest = plume_get_velocity_ptr((PlumeModel*)self->data);
    } else if (strcmp(name, "channel_outflow_end_water__depth") == 0) {
        *dest = plume_get_width_ptr((PlumeModel*)self->data);
    } else if (strcmp(name,
            "channel_outflow_end_suspended_load_sediment__volume_concentration") == 0) {
        *dest = plume_get_qs_ptr((PlumeModel*)self->data);
    } else if (strcmp(name, "sea_bottom_sediment__deposition_rate") == 0) {
        *dest = plume_get_deposition_rate_buffer((PlumeModel*)self->data);
    } else if (strcmp(name, "channel_outflow_end_bed_load_sediment__mass_flow_rate") == 0) {
        *dest = plume_get_bedload_ptr((PlumeModel*)self->data);
    } else if (strcmp(name, "channel_outflow_end__bankfull_width") == 0) {
        *dest = plume_get_width_ptr((PlumeModel*)self->data);
    } else {
        *dest = NULL;
        return BMI_FAILURE;
    }

    if (*dest) {
        return BMI_SUCCESS;
    } else {
        return BMI_FAILURE;
    }
}


static int
get_value(BMI_Model* self, const char* name, void* dest)
{
    void* src = NULL;
    int nbytes = 0;

    if (get_value_ptr(self, name, &src) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    if (self->get_var_nbytes(self, name, &nbytes) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    memcpy(dest, src, nbytes);

    return BMI_SUCCESS;
}


static int
get_value_at_indices(BMI_Model* self, const char* name, void* dest,
    int* inds, int len)
{
    void* src = NULL;
    int itemsize = 0;

    if (get_value_ptr(self, name, &src) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    if (self->get_var_itemsize(self, name, &itemsize) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    { /* Copy the data */
        int i;
        int offset;
        char* ptr;

        for (i = 0, ptr = (char*)dest; i < len; i++, ptr += itemsize) {
            offset = inds[i] * itemsize;
            memcpy(ptr, (char*)src + offset, itemsize);
        }
    }

    return BMI_SUCCESS;
}


static int
set_value(BMI_Model* self, const char* name, void* array)
{
    void* dest = NULL;
    int nbytes = 0;

    if (get_value_ptr(self, name, &dest) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    if (self->get_var_nbytes(self, name, &nbytes) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    memcpy(dest, array, nbytes);

    return BMI_SUCCESS;
}


static int
set_value_at_indices(BMI_Model* self, const char* name, int* inds, int len,
    void* src)
{
    void* to = NULL;
    int itemsize = 0;

    if (get_value_ptr(self, name, &to) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    if (self->get_var_itemsize(self, name, &itemsize) == BMI_FAILURE) {
        return BMI_FAILURE;
    }

    { /* Copy the data */
        int i;
        int offset;
        char* ptr;

        for (i = 0, ptr = (char*)src; i < len; i++, ptr += itemsize) {
            offset = inds[i] * itemsize;
            memcpy((char*)to + offset, ptr, itemsize);
        }
    }

    return BMI_SUCCESS;
}


BMI_Model*
register_bmi_plume(BMI_Model* model)
{
    model->data = NULL;

    model->initialize = initialize;
    model->update = update;
    model->update_until = update_until;
    model->finalize = finalize;

    model->get_component_name = get_component_name;
    model->get_input_item_count = get_input_item_count;
    model->get_output_item_count = get_output_item_count;
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
