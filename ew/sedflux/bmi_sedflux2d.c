#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "bmi.h"

/* Implement this: Add model-specific includes */
#include "utils/utils.h"
#include "sedflux_api.h"


static int
get_component_name (BMI_Model *self, char * name)
{
    strncpy (name, "sedflux2d", BMI_MAX_COMPONENT_NAME);
    return BMI_SUCCESS;
}


#define INPUT_VAR_NAME_COUNT (9)
static const char *input_var_names[INPUT_VAR_NAME_COUNT] = {
    "bedrock_surface__elevation",
    "sea_bottom_sediment__increment_of_thickness",
    "bedrock_surface__increment_of_elevation",
    //"channel_exit_water__volume_flow_rate",
    "channel_exit_water_sediment~suspended__mass_concentration",
    "channel_exit_water_sediment~bedload__mass_flow_rate",
    "channel_exit_water_x-section__width",
    "channel_exit_water_x-section__depth",
    "channel_exit_water_flow__speed",
    "sea_water_surface__elevation",
};


static int
get_input_var_name_count(BMI_Model *self, int *count)
{
    *count = INPUT_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}


static int
get_input_var_names(BMI_Model *self, char **names)
{
    int i;
    for (i=0; i<INPUT_VAR_NAME_COUNT; i++) {
        strncpy(names[i], input_var_names[i], BMI_MAX_VAR_NAME);
    }
    return BMI_SUCCESS;
}


#define OUTPUT_VAR_NAME_COUNT (33)
static const char *output_var_names[OUTPUT_VAR_NAME_COUNT] = {
    "land-or-seabed_sediment_grain__mean_diameter",
    "sea_water__depth",
    "sea_bottom_sediment__bulk_mass-per-volume_density",
    "sea_bottom_surface__elevation",
    "sea_bottom_sediment_grain__mean_diameter",
    "bedrock_surface__elevation",
    "land-or-seabed_sediment__permeability",
    "sediment_grain__mean_diameter",
    "land-or-seabed_sediment_surface__y_derivative_of_elevation",
    "sea_bottom_sediment__porosity",
    "land-or-seabed_sediment_silt__volume_fraction",
    "land-or-seabed_sediment_surface__elevation",
    "land-or-seabed_sediment_clay__volume_fraction",
    "sea_bottom_sediment_mud__volume_fraction",
    "surface_sediment_model_grain_class_0__volume_fraction",
    "land-or-seabed_sediment_sand__volume_fraction",
    "land-or-seabed_sediment__mean_of_deposition_age",
    "sediment__mean_of_deposition_age",
    "sea_bottom_sediment_model_grain_class_0__volume_fraction",
    "sea_bottom_surface__y_derivative_of_elevation",
    "sea_bottom_sediment_clay__volume_fraction",
    "land-or-seabed_sediment__porosity",
    "land-or-seabed_sediment__bulk_density",
    "land-or-seabed_sediment_mud__volume_fraction",
    "land-or-seabed_sediment_surface__x_derivative_of_elevation",
    "sediment__porosity",
    "sediment__bulk_mass-per-volume_density",
    "sea_bottom_sediment__permeability",
    "sediment__permeability",
    "sea_bottom_surface__x_derivative_of_elevation",
    "sea_bottom_sediment_sand__volume_fraction",
    "sea_bottom_sediment__mean_of_deposition_age",
    "sea_bottom_sediment_silt__volume_fraction"
};


static int
get_output_var_name_count(BMI_Model *self, int *count)
{
    *count = OUTPUT_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}


static int
get_output_var_names(BMI_Model *self, char **names)
{
    int i;
    for (i=0; i<OUTPUT_VAR_NAME_COUNT; i++) {
        strncpy(names[i], output_var_names[i], BMI_MAX_VAR_NAME);
    }
    return BMI_SUCCESS;
}


static int
get_start_time(BMI_Model *self, double *time)
{
    *time = 0.;
    return BMI_SUCCESS;
}


static int
get_end_time(BMI_Model *self, double *time)
{ /* Implement this: Set end time */
    *time = sedflux_get_end_time((Sedflux_state*)self->data) * 365.;
    return BMI_SUCCESS;
}


static int
get_current_time(BMI_Model *self, double *time)
{ /* Implement this: Set current time */
    *time = sedflux_get_current_time((Sedflux_state*)self->data) * 365.;
    return BMI_SUCCESS;
}


static int
get_time_step(BMI_Model *self, double *dt)
{ /* Implement this: Set time step */
    *dt = 1.;
    return BMI_SUCCESS;
}


static int
get_time_units(BMI_Model *self, char *units)
{
    strncpy(units, "d", BMI_MAX_UNITS_NAME);
    return BMI_SUCCESS;
}


static int
initialize(BMI_Model *self, const char * file)
{ /* Implement this: Create and initialize a model handle */
/*
    if (!g_thread_get_initialized()) {
        g_thread_init(NULL);
        eh_init_glib();
        g_log_set_handler(NULL, G_LOG_LEVEL_MASK, &eh_logger, NULL);
    }
*/
    g_log_set_handler(NULL, G_LOG_LEVEL_MASK, &eh_logger, NULL);

    {
        Sedflux_state * data = NULL;
        char *args = NULL;

        args = g_strdup_printf("sedflux -2 -i %s", file);

#if 0
        if (file) { /* Read the config file */
            FILE * fp = fopen (file, "r");

            if (fp)
            {
                char line[2048];
                if (fgets(line, 2048, fp) != line) {
                    return BMI_FAILURE;
                }
                args = g_strdup(line);
            }
            else
                return BMI_FAILURE; // Unable to open file
        }
        else
            return BMI_FAILURE;
#endif

        if (args) { /* Initialize with these argments */
            int argc;
            char **argv;
            int i;

            argv = g_strsplit(args, " ", 0);
            argc = g_strv_length(argv);

            for (i=0; i<argc; i++)
                g_strstrip (argv[i]);

            data = sedflux_initialize(argc, (const char**)argv);

            g_strfreev(argv);

            if (data)
                self->data = data;
            else
                return BMI_FAILURE;

            //set_input_var_names(*handle, input_var_names);
            //set_output_var_names(*handle, output_var_names);
        }
        else
            return BMI_FAILURE;
    }
  
    return BMI_SUCCESS;
}


#if 0
static int
update_frac(BMI_Model *self, double f)
{ /* Implement this: Update for a fraction of a time step */
    return BMI_FAILURE;
}
#endif


static int
update(BMI_Model *self)
{
    sedflux_run_time_step((Sedflux_state*)self->data);
    return BMI_SUCCESS;
}


static int
update_until(BMI_Model *self, double then)
{
    sedflux_run_until((Sedflux_state*)self->data, then / 365.);
    return BMI_SUCCESS;
}


static int
finalize(BMI_Model *self)
{ /* Implement this: Clean up */
    sedflux_finalize((Sedflux_state*)self->data);
    return BMI_SUCCESS;
}


static int
get_grid_type(BMI_Model *self, int id, char *type)
{
    if (id == 0) {
        strncpy(type, "uniform_rectilinear", 2048);
    } else if (id == 1) {
        strncpy(type, "uniform_rectilinear", 2048);
    } else if (id == 2) {
        strncpy(type, "scalar", 2048);
    } else {
        type[0] = '\0'; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_grid_rank(BMI_Model *self, int id, int *rank)
{
    if (id == 0) {
        *rank = 1;
    } else if (id == 1) {
        *rank = 2;
    } else if (id == 2) {
        *rank = 0;
    } else {
        *rank = -1; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_grid_shape(BMI_Model *self, int id, int *shape)
{ /* Implement this: set shape of structured grids */
    if (id == 0) {
        shape[0] = sedflux_get_ny((Sedflux_state*)self->data);
    } else if (id == 1) {
        shape[0] = sedflux_get_ny((Sedflux_state*)self->data);
        shape[1] = sedflux_get_nz((Sedflux_state*)self->data);
    } else {
        return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_grid_size(BMI_Model *self, int id, int *size)
{
    int rank;
    if (self->get_grid_rank(self, id, &rank) == BMI_FAILURE)
        return BMI_FAILURE;

    if (rank == 0) {
        *size = 1;
    } else {
        int * shape = (int*) malloc(sizeof(int) * rank);
        int i;

        if (self->get_grid_shape(self, id, shape) == BMI_FAILURE)
            return BMI_FAILURE;

        *size = 1;
        for (i=0; i<rank; i++)
            *size *= shape[i];
        free(shape);
    }

    return BMI_SUCCESS;
}


static int
get_grid_spacing(BMI_Model *self, int id, double *spacing)
{ /* Implement this: set spacing of uniform rectilinear grids */
    if (id == 0) {
        spacing[0] = sedflux_get_yres((Sedflux_state*)self->data);
    } else if (id == 1) {
        spacing[0] = sedflux_get_yres((Sedflux_state*)self->data);
        spacing[1] = sedflux_get_zres((Sedflux_state*)self->data);
    } else {
        return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_grid_origin(BMI_Model *self, int id, double *origin)
{ /* Implement this: set origin of uniform rectilinear grids */
    if (id == 0) {
        origin[0] = 0.;
    } else if (id == 1) {
        origin[0] = 0.; origin[1] = 0.;
    } else {
        return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_grid(BMI_Model *self, const char *name, int *grid)
{
    if (strcmp(name, "land-or-seabed_sediment_grain__mean_diameter") == 0) {
        *grid = 0;
    } else if (strcmp(name, "channel_exit_water_x-section__width") == 0 ||
               strcmp(name, "channel_exit_water_x-section__depth") == 0 ||
               strcmp(name, "channel_exit_water_sediment~suspended__mass_concentration") == 0 ||
               strcmp(name, "channel_exit_water_flow__speed") == 0 ||
               strcmp(name, "channel_exit_water_sediment~bedload__mass_flow_rate") == 0 ||
               strcmp(name, "sea_water_surface__elevation") == 0) {
        *grid = 2;
    } else if (strcmp(name, "sea_water__depth") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_bottom_sediment__bulk_mass-per-volume_density") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_bottom_surface__elevation") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_bottom_sediment_grain__mean_diameter") == 0) {
        *grid = 0;
    } else if (strcmp(name, "bedrock_surface__elevation") == 0) {
        *grid = 0;
    } else if (strcmp(name, "land-or-seabed_sediment__permeability") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sediment_grain__mean_diameter") == 0) {
        *grid = 1;
    } else if (strcmp(name, "land-or-seabed_sediment_surface__y_derivative_of_elevation") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_bottom_sediment__porosity") == 0) {
        *grid = 0;
    } else if (strcmp(name, "land-or-seabed_sediment_silt__volume_fraction") == 0) {
        *grid = 0;
    } else if (strcmp(name, "land-or-seabed_sediment_surface__elevation") == 0) {
        *grid = 0;
    } else if (strcmp(name, "land-or-seabed_sediment_clay__volume_fraction") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_bottom_sediment_mud__volume_fraction") == 0) {
        *grid = 0;
    } else if (strcmp(name, "surface_sediment_model_grain_class_0__volume_fraction") == 0) {
        *grid = 0;
    } else if (strcmp(name, "land-or-seabed_sediment_sand__volume_fraction") == 0) {
        *grid = 0;
    } else if (strcmp(name, "land-or-seabed_sediment__mean_of_deposition_age") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sediment__mean_of_deposition_age") == 0) {
        *grid = 1;
    } else if (strcmp(name, "sea_bottom_sediment_model_grain_class_0__volume_fraction") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_bottom_surface__y_derivative_of_elevation") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_bottom_sediment_clay__volume_fraction") == 0) {
        *grid = 0;
    } else if (strcmp(name, "land-or-seabed_sediment__porosity") == 0) {
        *grid = 0;
    } else if (strcmp(name, "land-or-seabed_sediment__bulk_density") == 0) {
        *grid = 0;
    } else if (strcmp(name, "land-or-seabed_sediment_mud__volume_fraction") == 0) {
        *grid = 0;
    } else if (strcmp(name, "land-or-seabed_sediment_surface__x_derivative_of_elevation") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_bottom_sediment__increment_of_thickness") == 0) {
        *grid = 0;
    } else if (strcmp(name, "bedrock_surface__increment_of_elevation") == 0) {
        *grid = 0;
    } else if (strcmp(name, "channel_exit_water__volume_flow_rate") == 0) {
        *grid = 2;
    } else if (strcmp(name, "sediment__porosity") == 0) {
        *grid = 1;
    } else if (strcmp(name, "sediment__bulk_mass-per-volume_density") == 0) {
        *grid = 1;
    } else if (strcmp(name, "sea_bottom_sediment__permeability") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sediment__permeability") == 0) {
        *grid = 1;
    } else if (strcmp(name, "sea_bottom_surface__x_derivative_of_elevation") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_bottom_sediment_sand__volume_fraction") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_bottom_sediment__mean_of_deposition_age") == 0) {
        *grid = 0;
    } else if (strcmp(name, "sea_bottom_sediment_silt__volume_fraction") == 0) {
        *grid = 0;
    } else {
        *grid = -1; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_type(BMI_Model *self, const char *name, char *type)
{
    if (strcmp(name, "land-or-seabed_sediment_grain__mean_diameter") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_exit_water_x-section__width") == 0 ||
               strcmp(name, "channel_exit_water_x-section__depth") == 0 ||
               strcmp(name, "channel_exit_water_sediment~suspended__mass_concentration") == 0 ||
               strcmp(name, "channel_exit_water_flow__speed") == 0 ||
               strcmp(name, "channel_exit_water_sediment~bedload__mass_flow_rate") == 0 ||
               strcmp(name, "sea_water_surface__elevation") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_water__depth") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment__bulk_mass-per-volume_density") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_surface__elevation") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment_grain__mean_diameter") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "bedrock_surface__elevation") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment__permeability") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sediment_grain__mean_diameter") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_surface__y_derivative_of_elevation") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment__porosity") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_silt__volume_fraction") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_surface__elevation") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_clay__volume_fraction") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment_mud__volume_fraction") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "surface_sediment_model_grain_class_0__volume_fraction") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_sand__volume_fraction") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment__mean_of_deposition_age") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sediment__mean_of_deposition_age") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment_model_grain_class_0__volume_fraction") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_surface__y_derivative_of_elevation") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment_clay__volume_fraction") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment__porosity") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment__bulk_density") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_mud__volume_fraction") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_surface__x_derivative_of_elevation") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment__increment_of_thickness") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "bedrock_surface__increment_of_elevation") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_exit_water__volume_flow_rate") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sediment__porosity") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sediment__bulk_mass-per-volume_density") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment__permeability") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sediment__permeability") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_surface__x_derivative_of_elevation") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment_sand__volume_fraction") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment__mean_of_deposition_age") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment_silt__volume_fraction") == 0) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
    } else {
        type[0] = '\0'; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_units(BMI_Model *self, const char *name, char *units)
{
    if (strcmp(name, "land-or-seabed_sediment_grain__mean_diameter") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_exit_water_x-section__width") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_exit_water_x-section__depth") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_exit_water_sediment~suspended__mass_concentration") == 0) {
        strncpy(units, "kg / m^3", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_exit_water_flow__speed") == 0) {
        strncpy(units, "m / s", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_water_surface__elevation") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_water__depth") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment__bulk_mass-per-volume_density") == 0) {
        strncpy(units, "kg / m^3", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_surface__elevation") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment_grain__mean_diameter") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "bedrock_surface__elevation") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment__permeability") == 0) {
        strncpy(units, "m^2", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sediment_grain__mean_diameter") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_surface__y_derivative_of_elevation") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment__porosity") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_silt__volume_fraction") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_exit_water_sediment~bedload__mass_flow_rate") == 0) {
        strncpy(units, "kg / s", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_surface__elevation") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_clay__volume_fraction") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment_mud__volume_fraction") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "surface_sediment_model_grain_class_0__volume_fraction") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_sand__volume_fraction") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment__mean_of_deposition_age") == 0) {
        strncpy(units, "d", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sediment__mean_of_deposition_age") == 0) {
        strncpy(units, "d", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment_model_grain_class_0__volume_fraction") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_surface__y_derivative_of_elevation") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment_clay__volume_fraction") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment__porosity") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment__bulk_density") == 0) {
        strncpy(units, "kg / m^3", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_mud__volume_fraction") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "land-or-seabed_sediment_surface__x_derivative_of_elevation") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment__increment_of_thickness") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "bedrock_surface__increment_of_elevation") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "channel_exit_water__volume_flow_rate") == 0) {
        strncpy(units, "m^3 / s", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sediment__porosity") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sediment__bulk_mass-per-volume_density") == 0) {
        strncpy(units, "kg / m^3", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment__permeability") == 0) {
        strncpy(units, "m^2", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sediment__permeability") == 0) {
        strncpy(units, "m^2", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_surface__x_derivative_of_elevation") == 0) {
        strncpy(units, "meter", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment_sand__volume_fraction") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment__mean_of_deposition_age") == 0) {
        strncpy(units, "d", BMI_MAX_UNITS_NAME);
    } else if (strcmp(name, "sea_bottom_sediment_silt__volume_fraction") == 0) {
        strncpy(units, "-", BMI_MAX_UNITS_NAME);
    } else {
        units[0] = '\0'; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_itemsize(BMI_Model *self, const char *name, int *itemsize)
{
    if (strcmp(name, "land-or-seabed_sediment_grain__mean_diameter") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "channel_exit_water_x-section__width") == 0 ||
               strcmp(name, "channel_exit_water_x-section__depth") == 0 ||
               strcmp(name, "channel_exit_water_sediment~suspended__mass_concentration") == 0 ||
               strcmp(name, "channel_exit_water_flow__speed") == 0 ||
               strcmp(name, "sea_water_surface__elevation") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_water__depth") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_sediment__bulk_mass-per-volume_density") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_surface__elevation") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_sediment_grain__mean_diameter") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "bedrock_surface__elevation") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "land-or-seabed_sediment__permeability") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sediment_grain__mean_diameter") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "land-or-seabed_sediment_surface__y_derivative_of_elevation") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_sediment__porosity") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "land-or-seabed_sediment_silt__volume_fraction") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "channel_exit_water_sediment~bedload__mass_flow_rate") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "land-or-seabed_sediment_surface__elevation") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "land-or-seabed_sediment_clay__volume_fraction") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_sediment_mud__volume_fraction") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "surface_sediment_model_grain_class_0__volume_fraction") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "land-or-seabed_sediment_sand__volume_fraction") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "land-or-seabed_sediment__mean_of_deposition_age") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sediment__mean_of_deposition_age") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_sediment_model_grain_class_0__volume_fraction") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_surface__y_derivative_of_elevation") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_sediment_clay__volume_fraction") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "land-or-seabed_sediment__porosity") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "land-or-seabed_sediment__bulk_density") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "land-or-seabed_sediment_mud__volume_fraction") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "land-or-seabed_sediment_surface__x_derivative_of_elevation") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_sediment__increment_of_thickness") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "bedrock_surface__increment_of_elevation") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "channel_exit_water__volume_flow_rate") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sediment__porosity") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sediment__bulk_mass-per-volume_density") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_sediment__permeability") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sediment__permeability") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_surface__x_derivative_of_elevation") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_sediment_sand__volume_fraction") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_sediment__mean_of_deposition_age") == 0) {
        *itemsize = sizeof(double);
    } else if (strcmp(name, "sea_bottom_sediment_silt__volume_fraction") == 0) {
        *itemsize = sizeof(double);
    } else {
        *itemsize = 0; return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_nbytes(BMI_Model *self, const char *name, int *nbytes)
{
    int id, size, itemsize;

    if (self->get_var_grid(self, name, &id) == BMI_FAILURE)
        return BMI_FAILURE;

    if (self->get_grid_size(self, id, &size) == BMI_FAILURE)
        return BMI_FAILURE;

    if (self->get_var_itemsize(self, name, &itemsize) == BMI_FAILURE)
        return BMI_FAILURE;

    *nbytes = itemsize * size;

    return BMI_SUCCESS;
}


static int
get_value(BMI_Model *self, const char *name, void *dest)
{
    const char *sedflux_name = NULL;

    if (strcmp(name, "sea_water__depth") == 0)
        sedflux_name = "DEPTH";
    else if (g_str_has_suffix(name, "surface__elevation"))
        sedflux_name = "ELEVATION";
    else if (g_str_has_suffix(name, "surface__x_derivative_of_elevation"))
        sedflux_name = "XSLOPE";
    else if (g_str_has_suffix(name, "surface__y_derivative_of_elevation"))
        sedflux_name = "YSLOPE";
    else if (g_str_has_suffix(name, "sediment_grain__mean_diameter"))
        sedflux_name = "GRAIN";
    else if (g_str_has_suffix(name, "sediment__mean_of_deposition_age"))
        sedflux_name = "AGE";
    else if (g_str_has_suffix(name, "sediment_sand__volume_fraction"))
        sedflux_name = "SAND";
    else if (g_str_has_suffix(name, "sediment_silt__volume_fraction"))
        sedflux_name = "SILT";
    else if (g_str_has_suffix(name, "sediment_clay__volume_fraction"))
        sedflux_name = "CLAY";
    else if (g_str_has_suffix(name, "sediment_mud__volume_fraction"))
        sedflux_name = "MUD";
    else if (g_str_has_suffix(name, "sediment__bulk_density"))
        sedflux_name = "DENSITY";
    else if (g_str_has_suffix(name, "sediment__porosity"))
        sedflux_name = "POROSITY";
    else if (g_str_has_suffix(name, "sediment__permeability"))
        sedflux_name = "PERMEABILITY";
    else if (strcmp(name, "bedrock_surface__elevation") == 0)
        sedflux_name = "BASEMENT";
    else if (g_str_has_suffix(name, "sediment_model_grain_class_0__volume_fraction"))
        sedflux_name = "";
    else
        return BMI_FAILURE;

    if (g_str_has_prefix(name, "sediment_")) {
        if (!sedflux_get_sediment_value((Sedflux_state*)self->data, sedflux_name, (double*)dest))
            return BMI_FAILURE;
    }
    else {
        gint mask = 0;

        if (g_str_has_prefix(name, "sea_bottom_"))
            mask |= MASK_LAND;

        if (!sedflux_get_surface_value((Sedflux_state*)self->data, sedflux_name, (double*)dest, mask))
            return BMI_FAILURE;
    }

  return BMI_SUCCESS;
}


#if 0
int
get_value(BMI_Model *self, const char * name, void *dest)
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
get_value_at_indices (BMI_Model *self, const char *name, void *dest,
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
#endif


static int
set_value (BMI_Model *self, const char *name, void *array)
{
    if (strcmp(name, "bedrock_surface__elevation") == 0)
        sedflux_set_basement((Sedflux_state*)self->data, (double*)array);
    else if (strcmp(name, "bedrock_surface__increment_of_elevation") == 0)
        sedflux_set_uplift((Sedflux_state*)self->data, (double*)array);
    else if (strcmp(name, "sea_bottom_sediment__increment_to_thickness") == 0) // Should this be sea_bottom_sediment__thickness???
        sedflux_set_subaerial_deposition_to((Sedflux_state*)self->data, (double*)array);
    else if (strcmp(name, "channel_exit_water__volume_flow_rate") == 0)
        sedflux_set_discharge((Sedflux_state*)self->data, (double*)array);
    else if (strcmp(name, "channel_exit_water_sediment~bedload__mass_flow_rate") == 0)
        sedflux_set_channel_bedload((Sedflux_state*)self->data, (double*)array);
        //sedflux_set_bed_load_flux((Sedflux_state*)self->data, (double*)array);
    else if (strcmp(name, "channel_exit_water_x-section__width") == 0)
        sedflux_set_channel_width((Sedflux_state*)self->data, (double*)array);
    else if (strcmp(name, "channel_exit_water_x-section__depth") == 0)
        sedflux_set_channel_depth((Sedflux_state*)self->data, (double*)array);
    else if (strcmp(name, "channel_exit_water_sediment~suspended__mass_concentration") == 0)
        sedflux_set_channel_suspended_load((Sedflux_state*)self->data, (double*)array);
    else if (strcmp(name, "channel_exit_water_flow__speed") == 0)
        sedflux_set_channel_velocity((Sedflux_state*)self->data, (double*)array);
    else if (strcmp(name, "sea_water_surface__elevation") == 0)
        sedflux_set_sea_level((Sedflux_state*)self->data, (double*)array);
    else
        return BMI_FAILURE;
    return BMI_SUCCESS;
}

#if 0
static int
set_value_at_indices (BMI_Model *self, const char *name, int * inds, int len,
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
#endif

BMI_Model*
register_bmi_sedflux2d(BMI_Model *model)
{
    model->data = NULL;

    model->initialize = initialize;
    model->update = update;
    model->update_until = update_until;
    // model->update_frac = NULL;
    model->finalize = finalize;
    // model->run_model = NULL;

    model->get_component_name = get_component_name;
    model->get_input_item_count = get_input_var_name_count;
    model->get_output_item_count = get_output_var_name_count;
    model->get_input_var_names = get_input_var_names;
    model->get_output_var_names = get_output_var_names;

    model->get_var_grid = get_var_grid;
    model->get_var_type = get_var_type;
    model->get_var_units = get_var_units;
    model->get_var_itemsize = get_var_itemsize;
    model->get_var_nbytes = get_var_nbytes;
    model->get_current_time = get_current_time;
    model->get_start_time = get_start_time;
    model->get_end_time = get_end_time;
    model->get_time_units = get_time_units;
    model->get_time_step = get_time_step;

    model->get_value = get_value;
    model->get_value_ptr = NULL;
    model->get_value_at_indices = NULL;

    model->set_value = set_value;
    // model->set_value_ptr = NULL;
    model->set_value_at_indices = NULL;

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
