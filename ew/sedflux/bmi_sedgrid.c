#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <glib.h>

#include "bmi.h"

/* Implement this: Add model-specific includes */
#include "utils/utils.h"
#include "sed/sed_cube.h"
#include "sed/sed_sediment.h"
#include "sedflux_api.h"


typedef struct {
    gchar* name;
    gchar* units;
    char* intent;
    gint grid;
    gchar* local_name;
    void (*set_func)(void*, double*);
} ExchangeItem;


typedef struct {
    gint id;
    gint rank;
    gchar *type;
} Grid;


static Grid grids[] = {
  {0, 2, "uniform_rectilinear"},
  {1, 3, "uniform_rectilinear"},
  {2, 0, "scalar"},
  {-1, 0, NULL},
};

static ExchangeItem exchange_items[] = {
  {"bedrock_surface__elevation", "m", "inout", 0, "BASEMENT", sedflux_set_basement},
  {"sea_bottom_sediment__increment_of_thickness", "m", "in", 0, NULL, sedflux_set_subaerial_deposition_to},
  {"bedrock_surface__increment_of_elevation", "m", "in", 0, NULL, sedflux_set_uplift},
  {"sea_water__depth", "m", "out", 0, "DEPTH", NULL},
  {"sea_bottom_surface__elevation", "m", "out", 0, "ELEVATION", NULL},
  {NULL, "", "", 0, "", NULL},
};


typedef struct {
    int shape[2];
    double spacing[3];
    int n_grains;
} SedGridParams;


static SedGridParams*
scan_sedgrid_params(const char* name)
{
    SedGridParams* params = NULL;

    if (name) {
        GError *error = NULL;
        GKeyFile *keys = g_key_file_new();

        g_key_file_load_from_file(keys, name, G_KEY_FILE_NONE, &error);
        if (error)
            return NULL;

        params = (SedGridParams*)malloc(sizeof(SedGridParams));

        if (params) {
            gint * shape = NULL;
            gdouble * spacing = NULL;
            gint n_grains = 0;
            gsize len = 0;

            n_grains = g_key_file_get_integer(keys, "sedgrid", "n_grains", &error);
            if (n_grains <= 0 || error)
                return NULL;

            shape = g_key_file_get_integer_list(keys, "sedgrid", "shape", &len, &error);
            if (!shape || len != 2 || error)
                return NULL;

            spacing = g_key_file_get_double_list(keys, "sedgrid", "spacing", &len, &error);
            if (!spacing || len != 3 || error)
                return NULL;

            params->shape[0] = shape[0];
            params->shape[1] = shape[1];
            params->spacing[0] = spacing[0];
            params->spacing[1] = spacing[1];
            params->spacing[2] = spacing[2];
            params->n_grains = n_grains;

            g_free(shape);
            g_free(spacing);
        }

        g_key_file_free(keys);
    }

    return params;
}


static Sed_cube
set_sedgrid_xyz(Sed_cube cube, double spacing[3]) {
    if (cube) {
        int i, j;
        Sed_column this_col = NULL;

        sed_cube_set_z_res(cube, spacing[0]);
        sed_cube_set_y_res(cube, spacing[1]);
        sed_cube_set_x_res(cube, spacing[2]);

        for (i = 0 ; i < sed_cube_n_x(cube) ; i++ )
            for (j = 0 ; j < sed_cube_n_y(cube) ; j++) {
                this_col = sed_cube_col_ij(cube, i, j);

                sed_column_set_x_position(this_col, i * spacing[0]);
                sed_column_set_y_position(this_col, j * spacing[1]);

                sed_column_set_base_height(this_col, 0.);
            }
    }

    return cube;
}


static const char *input_var_names[] = {
    "bedrock_surface__elevation",
    "sea_bottom_sediment__increment_of_thickness",
    "bedrock_surface__increment_of_elevation",
    NULL
};


static const char *output_var_names[] = {
    "bedrock_surface__elevation",
    "sea_water__depth",
    "sea_bottom_surface__elevation",
    NULL
};


static GHashTable *INPUT_VAR_NAMES = NULL;
static GHashTable *OUTPUT_VAR_NAMES = NULL;
static GHashTable *ALL_VAR_NAMES =  NULL;
static GHashTable *VAR_GRID =  NULL;
static GHashTable *VAR_UNITS =  NULL;
static GHashTable *VAR_LOCAL_NAME = NULL; 
static GHashTable *VAR_SET_FUNC = NULL; 
static GHashTable *GRID_RANK =  NULL;
static GHashTable *GRID_TYPE =  NULL;


static gboolean
is_var(gchar* name)
{
    return g_hash_table_lookup_extended(ALL_VAR_NAMES, name, NULL, NULL);
}


static gboolean
is_input_var(gchar* name)
{
    return g_hash_table_lookup_extended(INPUT_VAR_NAMES, name, NULL, NULL);
}


static gboolean
is_output_var(gchar* name)
{
    return g_hash_table_lookup_extended(OUTPUT_VAR_NAMES, name, NULL, NULL);
}


static void
setup_grid_tables()
{
  Grid* grid;

  GRID_RANK = g_hash_table_new(g_int_hash, g_int_equal);
  GRID_TYPE = g_hash_table_new(g_int_hash, g_int_equal);

  for (grid = grids ; grid->id >= 0 ; grid ++ ) {
    gint *rank = g_new(gint, 1);
    *rank = grid->rank;
    g_hash_table_insert(GRID_RANK, &(grid->id), rank);
    g_hash_table_insert(GRID_TYPE, &(grid->id), g_strdup(grid->type));
  }
}


static void
setup_var_tables()
{
  ExchangeItem* item;

  INPUT_VAR_NAMES = g_hash_table_new(g_str_hash, g_str_equal);
  OUTPUT_VAR_NAMES = g_hash_table_new(g_str_hash, g_str_equal);
  ALL_VAR_NAMES = g_hash_table_new(g_str_hash, g_str_equal);

  VAR_GRID = g_hash_table_new(g_str_hash, g_str_equal);
  VAR_UNITS = g_hash_table_new(g_str_hash, g_str_equal);
  VAR_LOCAL_NAME = g_hash_table_new(g_str_hash, g_str_equal);
  VAR_SET_FUNC = g_hash_table_new(g_str_hash, g_str_equal);

  for (item = exchange_items ; item->name ; item++ ) {
    g_hash_table_insert(ALL_VAR_NAMES, item->name, NULL);
    if (strcmp(item->intent, "in") == 0) {
        g_hash_table_insert(INPUT_VAR_NAMES, item->name, NULL);
    } else if (strcmp(item->intent, "out") == 0) {
        g_hash_table_insert(OUTPUT_VAR_NAMES, item->name, NULL);
    } else if (strcmp(item->intent, "inout") == 0) {
        g_hash_table_insert(INPUT_VAR_NAMES, item->name, NULL);
        g_hash_table_insert(OUTPUT_VAR_NAMES, item->name, NULL);
    }

    {
      gchar* units = g_strdup(item->units);
      g_hash_table_insert(VAR_UNITS, item->name, units);
    }
    {
      gint* grid = g_new(gint, 1);
      *grid = item->grid;
      g_hash_table_insert(VAR_GRID, item->name, grid);
    }
    if (is_output_var(item->name)) {
      gchar* local = g_strdup(item->local_name);
      g_hash_table_insert(VAR_LOCAL_NAME, item->name, local);
    }
    if (is_input_var(item->name)) {
      g_hash_table_insert(VAR_SET_FUNC, item->name, item->set_func);
    }
  }
}


static int
get_component_name (void *self, char * name)
{
    strncpy (name, "sedgrid", BMI_MAX_COMPONENT_NAME);
    return BMI_SUCCESS;
}

/*
#define INPUT_VAR_NAME_COUNT (3)
static const char *input_var_names[INPUT_VAR_NAME_COUNT] = {
    "bedrock_surface__elevation",
    "sea_bottom_sediment__increment_of_thickness",
    "bedrock_surface__increment_of_elevation",
};
*/

static int
get_input_var_name_count(void *self, int *count)
{
    *count = g_hash_table_size(INPUT_VAR_NAMES);
    return BMI_SUCCESS;
}


static int
get_input_var_names(void *self, char **names)
{
    GHashTableIter iter;
    gpointer key;
    char **name = names;

    g_hash_table_iter_init(&iter, INPUT_VAR_NAMES);
    while (g_hash_table_iter_next(&iter, &key, NULL)) {
        strncpy(*name, (const char*)key, BMI_MAX_VAR_NAME);
        name ++;
    }

    return BMI_SUCCESS;
}

/*
#define OUTPUT_VAR_NAME_COUNT (4)
static const char *output_var_names[OUTPUT_VAR_NAME_COUNT] = {
    "bedrock_surface__elevation",
    "sea_water__depth",
    "sea_bottom_surface__elevation",
    "land-or-seabed_sediment_surface__elevation",
};
*/

static int
get_output_var_name_count(void *self, int *count)
{
    *count = g_hash_table_size(OUTPUT_VAR_NAMES);
    return BMI_SUCCESS;
}


static int
get_output_var_names(void *self, char **names)
{
    GHashTableIter iter;
    gpointer key;
    char **name = names;

    g_hash_table_iter_init(&iter, OUTPUT_VAR_NAMES);
    while (g_hash_table_iter_next(&iter, &key, NULL)) {
        strncpy(*name, (const char*)key, BMI_MAX_VAR_NAME);
        name ++;
    }

    return BMI_SUCCESS;
}


static int
get_start_time(void * self, double *time)
{
    *time = 0.;
    return BMI_SUCCESS;
}


static int
get_end_time(void * self, double *time)
{ /* Implement this: Set end time */
    *time = G_MAXDOUBLE;
    return BMI_SUCCESS;
}


static int
get_current_time(void * self, double *time)
{ /* Implement this: Set current time */
    *time = sed_cube_age_in_years((Sed_cube)self) * 365.;
    return BMI_SUCCESS;
}


static int
get_time_step(void * self, double *dt)
{ /* Implement this: Set time step */
    *dt = -1.;
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
    *handle = NULL;

    if (!g_thread_get_initialized()) {
        g_thread_init(NULL);
        eh_init_glib();
        g_log_set_handler(NULL, G_LOG_LEVEL_MASK, &eh_logger, NULL);
    }

    {
        Sed_cube cube = NULL;
        Sed_sediment sediment = NULL;
        SedGridParams* params = scan_sedgrid_params(file);
        int n;

        if (!params)
            return BMI_FAILURE;
        cube = sed_cube_new(params->shape[0], params->shape[1]);
        if (!cube)
            return BMI_FAILURE;

        set_sedgrid_xyz(cube, params->spacing);

        /* Set the sediment */
        sediment = sed_sediment_new();
        for (n = 0 ; n < params->n_grains ; n ++ ) {
            sed_sediment_add_type(sediment, sed_type_new());
        }

        sed_sediment_set_env(sediment);
        sed_sediment_destroy(sediment);

        *handle = (void*)cube;
    }
  
    return BMI_SUCCESS;
}


#if 0
static int
update_frac(void * self, double f)
{ /* Implement this: Update for a fraction of a time step */
    return BMI_FAILURE;
}
#endif


static int
update(void * self)
{
    return BMI_SUCCESS;
}


static int
update_until(void * self, double then)
{
    sed_cube_set_age((Sed_cube)self, then * 365.);
    return BMI_SUCCESS;
}


static int
finalize(void * self)
{ /* Implement this: Clean up */
    sed_cube_destroy((Sed_cube)self);
    return BMI_SUCCESS;
}


static int
get_grid_type(void *self, int id, char *type)
{
    char* gtype = g_hash_table_lookup(GRID_TYPE, &id);
    if (gtype) {
        strncpy(type, gtype, 2048);
        return BMI_SUCCESS;
    } else {
        type[0] = '\0';
        return BMI_FAILURE;
    }
}


static int
get_grid_rank(void *self, int id, int *rank)
{
    gint* value = g_hash_table_lookup(GRID_RANK, &id);
    if (value) {
        *rank = *value;
        return BMI_SUCCESS;
    } else {
        *rank = -1;
        return BMI_FAILURE;
    }
}


static int
get_grid_shape(void *self, int id, int *shape)
{ /* Implement this: set shape of structured grids */
    if (id == 0) {
        shape[0] = sed_cube_n_x((Sed_cube)self);
        shape[1] = sed_cube_n_y((Sed_cube)self);
    } else if (id == 1) {
        shape[0] = sed_cube_n_x((Sed_cube)self);
        shape[1] = sed_cube_n_y((Sed_cube)self);
        shape[2] = sed_cube_n_rows((Sed_cube)self);
    } else {
        return BMI_FAILURE;
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
{ /* Implement this: set spacing of uniform rectilinear grids */
    if (id == 0) {
        spacing[0] = sed_cube_x_res(self);
        spacing[1] = sed_cube_y_res(self);
    } else if (id == 1) {
        spacing[0] = sed_cube_x_res(self);
        spacing[1] = sed_cube_y_res(self);
        spacing[2] = sed_cube_z_res(self);
    } else {
        return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_grid_origin(void *self, int id, double *origin)
{ /* Implement this: set origin of uniform rectilinear grids */
    if (id == 0) {
        origin[0] = 0.;
        origin[1] = 0.;
    } else if (id == 1) {
        origin[0] = 0.;
        origin[1] = 0.;
        origin[2] = 0.;
    } else {
        return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_grid(void *self, const char *name, int *grid)
{
    int *id = g_hash_table_lookup(VAR_GRID, name);
    if (id) {
        *grid = *id;
        return BMI_SUCCESS;
    } else {
        return BMI_FAILURE;
    }
}


static int
get_var_type(void *self, const char *name, char *type)
{
    if (is_var(name)) {
        strncpy(type, "double", BMI_MAX_UNITS_NAME);
        return BMI_SUCCESS;
    } else {
        type[0] = '\0';
        return BMI_FAILURE;
    }
}


static int
get_var_units(void *self, const char *name, char *units)
{
    char *ustr = g_hash_table_lookup(VAR_UNITS, name);
    if (ustr) {
        strncpy(units, ustr, BMI_MAX_UNITS_NAME);
        return BMI_SUCCESS;
    } else {
        units[0] = '\0';
        return BMI_FAILURE;
    }
    return BMI_SUCCESS;
}


static int
get_var_itemsize(void *self, const char *name, int *itemsize)
{
    if (is_var(name)) {
        *itemsize = sizeof(double);
        return BMI_SUCCESS;
    } else {
        *itemsize = 0;
        return BMI_FAILURE;
    }
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
get_value(void *self, const char *name, void *dest)
{
    int status = BMI_FAILURE;

    if (is_output_var(name)) {
        gchar* sedflux_name = g_hash_table_lookup(VAR_LOCAL_NAME, name);
        if (sedflux_get_surface_value(self, sedflux_name, (double*)dest, 0))
            status = BMI_SUCCESS;
    }

    return status;
}


#if 0
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
#endif


static int
set_value (void *self, const char *name, void *array)
{
    int status = BMI_FAILURE;

    if (is_input_var(name)) {
        void (*func)(void*, double*);

        func = g_hash_table_lookup(VAR_SET_FUNC, name);
        if (func) {
            func(self, (double*)array);
            status = BMI_SUCCESS;
        }
    }

    return status;
}

#if 0
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
#endif

BMI_Model*
register_bmi_sedgrid(BMI_Model *model)
{
    setup_grid_tables();
    setup_var_tables();

    model->self = NULL;

    model->initialize = initialize;
    model->update = update;
    model->update_until = update_until;
    model->update_frac = NULL;
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
    model->get_current_time = get_current_time;
    model->get_start_time = get_start_time;
    model->get_end_time = get_end_time;
    model->get_time_units = get_time_units;
    model->get_time_step = get_time_step;

    model->get_value = get_value;
    model->get_value_ptr = NULL;
    model->get_value_at_indices = NULL;

    model->set_value = set_value;
    model->set_value_ptr = NULL;
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
