#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "utils/utils.h"

#include "avulsion.h"
#include "avulsion_api.h"

#ifdef SWIG
% include avulsion_api.h
#endif


AvulsionModel* _avulsion_alloc (void);
AvulsionModel * _avulsion_free (AvulsionModel * self);
int _avulsion_initialize (AvulsionModel* s);
int _avulsion_run_until (AvulsionModel* s, int until);
int _avulsion_finalize (AvulsionModel* s);

void avulsion_init_state (AvulsionModel* s);
void avulsion_free_state (AvulsionModel* s);

#define BMI_FAILURE_BAD_ARGUMENT_ERROR (2)
#define BMI_FAILURE_UNKNOWN_ERROR (3)
#define BMI_FAILURE_UNABLE_TO_OPEN_ERROR (4)
#define BMI_FAILURE_BAD_NAME_ERROR (5)

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

    new_list = (const char **) malloc (sizeof (char*) * count);
    for (i=0; i<count; i++) {
      new_list[i] = strdup (names[i]);
    }
  }

  *len = count;

  return new_list;
}

void
set_input_var_names (AvulsionModel *self) {
  const char *input_var_names[] = {
    // Scalars
    "avulsion_model__random_walk_variance_constant",
    "avulsion_model__sediment_bed_load_exponent",
    "avulsion_model__water_discharge_exponent",
    "channel_inflow_end_water__discharge",
    "channel_inflow_end_bed_load_sediment__mass_flow_rate",

    // Vectors
    "channel_outflow_end_bed_load_sediment__mass_flow_rate",
    "channel_outflow_end_water__discharge",
    "channel_outflow_end__location_model_x_component",
    "channel_outflow_end__location_model_y_component",

    // Grids
    "surface__elevation",
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

void
set_output_var_names (AvulsionModel *self) {
  int count;
  const char *output_var_names[] = {
    // Scalars
    "avulsion_model__random_walk_variance_constant",
    "avulsion_model__sediment_bed_load_exponent",
    "avulsion_model__water_discharge_exponent",
    "channel_inflow_end_water__discharge",
    "channel_inflow_end_bed_load_sediment__mass_flow_rate",

    // Vectors
    "channel_outflow_end_bed_load_sediment__mass_flow_rate",
    "channel_outflow_end_water__discharge",
    "channel_outflow_end__location_model_x_component",
    "channel_outflow_end__location_model_y_component",
    "channel_inflow_end_to_channel_outflow_end__angle",

    // Grids
    "surface__elevation",
    "surface_bed_load_sediment__mass_flow_rate",
    NULL
  };

  {
    int count;
    const char **names = dup_null_terminated_list (output_var_names, &count);

    self->output_var_name_count = count;
    self->output_var_names = names;
  }

  return;
}

int
has_input_var (AvulsionModel *self, const char *name) {
  const char **names = self->input_var_names;
  const int count = self->input_var_name_count;
  int i;

  for (i=0; i<count; ++i)
    if (strcmp (names[i], name) == 0)
      return TRUE;
  return FALSE;
}

int
has_output_var (AvulsionModel *self, const char *name) {
  const char **names = self->output_var_names;
  const int count = self->output_var_name_count;
  int i;

  for (i=0; i<count; ++i)
    if (strcmp (names[i], name) == 0)
      return TRUE;
  return FALSE;
}

int
BMI_AVULSION_Get_var_rank (void *self, const char * name, int *rank)
{
  if (rank) {
    if (strcmp (name, "avulsion_model__random_walk_variance_constant")==0 ||
        strcmp (name, "avulsion_model__sediment_bed_load_exponent")==0 ||
        strcmp (name, "avulsion_model__water_discharge_exponent")==0 ||
        strcmp (name, "channel_inflow_end_water__discharge")==0 ||
        strcmp (name, "channel_inflow_end_bed_load_sediment__mass_flow_rate")==0)
      *rank = 0;
    else if (strcmp (name, "channel_outflow_end_bed_load_sediment__mass_flow_rate")==0 ||
        strcmp (name, "channel_outflow_end_water__discharge")==0 ||
        strcmp (name, "channel_outflow_end__location_model_x_component")==0 ||
        strcmp (name, "channel_outflow_end__location_model_y_component")==0 ||
        strcmp (name, "channel_inflow_end_to_channel_outflow_end__angle")==0)
      *rank = 1;
    else if (strcmp (name, "surface__elevation")==0 ||
             strcmp (name, "surface_bed_load_sediment__mass_flow_rate")==0)
      *rank = 2;
    else
      return BMI_FAILURE;
  }
  else
    return BMI_FAILURE;

  return BMI_SUCCESS;
}

int
BMI_AVULSION_Initialize (const char *config_file, void **handle)
{
  int rtn = BMI_FAILURE;

  if (!g_thread_get_initialized ()) {
    g_thread_init (NULL);
    eh_init_glib ();
    g_log_set_handler (NULL, G_LOG_LEVEL_MASK, &eh_logger, NULL);
  }

  if (handle) {
    AvulsionModel *self = avulsion_init (NULL);

    if (self) {
      int shape[2] = {30, 40};
      double spacing[2] = {1, 1};
      int hinge_indices[2] = {0, 20};
      double angle_limit[2] = {60., 120.};
      double variance = 10.;
      double bed_load_exponent = 1.;
      double discharge_exponent = 1.;
      int number_of_river_mouths = 9;

      if (config_file) {
        FILE *fp = fopen (config_file, "r");

        if (fp) {
          fscanf (fp, "%d, %d\n", &shape[1], &shape[0]);
          fscanf (fp, "%lf, %lf\n", &spacing[1], &spacing[0]);
          fscanf (fp, "%d, %d\n", &hinge_indices[1], &hinge_indices[0]);
          fscanf (fp, "%lf, %lf\n", &angle_limit[0], &angle_limit[1]);
          fscanf (fp, "%lf\n", &variance);
          fscanf (fp, "%lf\n", &bed_load_exponent);
          fscanf (fp, "%lf\n", &discharge_exponent);
          fscanf (fp, "%d\n", &number_of_river_mouths);
        }
        else
          return BMI_FAILURE_UNABLE_TO_OPEN_ERROR;
      }
      //else
      //  return BMI_FAILURE;

      angle_limit[0] *= 3.14/180.;
      angle_limit[1] *= 3.14/180.;
      variance *= 3.14/180.;

      avulsion_set_grid (self, shape, spacing);
      avulsion_set_river_hinge (self, hinge_indices);
      avulsion_set_river_angle_limit (self, angle_limit);

      avulsion_set_variance (self, variance);
      avulsion_set_bed_load_exponent (self, bed_load_exponent);
      avulsion_set_discharge_exponent (self, discharge_exponent);
      avulsion_set_total_river_mouths (self, number_of_river_mouths);
      avulsion_set_discharge (self, 1000.);
      avulsion_set_sed_flux (self, 250.);

      set_input_var_names (self);
      set_output_var_names (self);

      *handle = (void *)self;

      rtn = BMI_SUCCESS;
    }
    else
      rtn = BMI_FAILURE_UNKNOWN_ERROR;
  }
  else
    rtn = BMI_FAILURE_BAD_ARGUMENT_ERROR;

  return rtn;
}

int
BMI_AVULSION_Get_component_name (void *self, char *name)
{
  if (name) {
    strcpy (name, BMI_AVULSION_COMPONENT_NAME);
    return BMI_SUCCESS;
  }
  else
    return BMI_FAILURE;
}

int
BMI_AVULSION_Get_start_time (void *self, double *time)
{
  if (time) {
    *time = avulsion_get_start_time ((AvulsionModel *)self);
    return BMI_SUCCESS;
  }
  else
    return BMI_FAILURE;
}

int
BMI_AVULSION_Get_current_time (void *self, double *time)
{
  if (time) {
    *time = avulsion_get_current_time ((AvulsionModel*)self);
    return BMI_SUCCESS;
  }
  else
    return BMI_FAILURE;
}

int
BMI_AVULSION_Get_end_time (void *self, double *time)
{
  if (time) {
    *time = avulsion_get_end_time ((AvulsionModel*)self);
    return BMI_SUCCESS;
  }
  else
    return BMI_FAILURE;
}

int
BMI_AVULSION_Get_time_step (void *self, double *dt)
{
  eh_return_val_if_fail (self && dt, BMI_FAILURE);

  *dt = ((AvulsionModel*)self)->time_step;
  return BMI_SUCCESS;
}

int
BMI_AVULSION_Get_time_units (void *self, char *units)
{
  eh_return_val_if_fail (self && units, BMI_FAILURE);

  strcpy (units, "d");
  return BMI_SUCCESS;
}

int
BMI_AVULSION_Update_until (void *self, double time_in_days)
{
  int until_time_step = time_in_days / ((AvulsionModel*)self)->time_step;
  _avulsion_run_until ((AvulsionModel*)self, until_time_step);
  return BMI_SUCCESS;
}

int
BMI_AVULSION_Update (void *self)
{
  double now;
  if (BMI_AVULSION_Get_current_time (self, &now) == BMI_SUCCESS &&
      BMI_AVULSION_Update_until (self, now + ((AvulsionModel*)self)->time_step) == BMI_SUCCESS)
    return BMI_SUCCESS;
  else
    return BMI_FAILURE;
}

int
BMI_AVULSION_Finalize (void *self)
{
  if (self) {
    _avulsion_finalize ((AvulsionModel *)self);
    _avulsion_free ((AvulsionModel *)self);
  }

  return BMI_SUCCESS;
}

AvulsionModel *
_avulsion_alloc (void)
{
  AvulsionModel *s = g_new (AvulsionModel, 1);

  avulsion_init_state (s);

  return s;
}

AvulsionModel*
_avulsion_free (AvulsionModel * self)
{
  if (self)
  {
    avulsion_free_state (self);
    g_free (self);
  }
  return NULL;
}

AvulsionModel *
avulsion_init (AvulsionModel * self)
{
  if (!self)
    self = _avulsion_alloc ();

  _avulsion_initialize (self);

  return self;
}

int
avulsion_run_until (AvulsionModel * self, double time_in_days)
{
  int until_time_step = time_in_days / self->time_step;
  return _avulsion_run_until (self, until_time_step);
}

AvulsionModel *
avulsion_finalize (AvulsionModel * self, int free)
{
  _avulsion_finalize (self);

  if (free)
    self = _avulsion_free (self);

  return self;
}

AvulsionModel *
avulsion_set_variance (AvulsionModel * self, double variance)
{
  self->variance = variance;

  {
    Sed_riv r = sed_cube_borrow_nth_river (self->p, 0);
    Avulsion_st* data;

    data = avulsion_new (
             (self->seed==0)?g_rand_new():g_rand_new_with_seed(self->seed),
             variance);
    eh_require (data);
    sed_river_set_avulsion_data (r, data);
  }
  return self;
}

AvulsionModel *
avulsion_set_dx (AvulsionModel * self, double dx)
{
  self->dx = dx;
  return self;
}

AvulsionModel *
avulsion_set_dy (AvulsionModel * self, double dy)
{
  self->dy = dy;
  return self;
}

void
_avulsion_update_elevation (AvulsionModel* self)
{
  {
    int i;
    const int len = sed_cube_size (self->p);
    for (i=0; i<len; i++)
    {
      self->elevation[i] = sed_cube_elevation (self->p, 0, i);
    }
  }

  return;
}

AvulsionModel*
avulsion_set_grid (AvulsionModel* self, gint shape[2], double res[2])
{
  if (!self->p)
  {
    Sed_riv r = sed_river_new ("AvulsionRiver1");

    { /* Create sediment */
      Sed_sediment s = NULL;
      GError* error = NULL;
      gchar* buffer = sed_sediment_default_text ();

      s = sed_sediment_scan_text (buffer, &error);
      g_free (buffer);

      eh_print_on_error (error, "%s: Error creating sediment.", "Avulsion");
      sed_sediment_set_env (s);
    }

    self->p = sed_cube_new (shape[1], shape[0]);
    sed_cube_set_z_res (self->p, 1.);
    sed_cube_set_y_res (self->p, res[0]);
    sed_cube_set_x_res (self->p, res[1]);

    sed_cube_add_trunk (self->p, r);

    self->dy = res[0];
    self->dx = res[1];
    self->nx = sed_cube_n_x (self->p);
    self->ny = sed_cube_n_y (self->p);
    self->elevation = g_new (double, sed_cube_size (self->p));
    self->discharge = g_new (double, sed_cube_size (self->p));

    _avulsion_update_elevation (self);
  }
  return self;
}

AvulsionModel*
avulsion_set_river_hinge (AvulsionModel* self, gint ind[2])
{
  Sed_riv r = sed_cube_borrow_nth_river (self->p, 0);
  sed_river_set_hinge (r, ind[1], ind[0]);
  //sed_river_set_hinge (r, ind[0], ind[1]);
  return self;
}

AvulsionModel*
avulsion_set_river_angle_limit (AvulsionModel* self, double limit[2])
{
  Sed_riv r = sed_cube_borrow_nth_river (self->p, 0);
  sed_river_set_angle_limit (r, limit[0], limit[1]);
  sed_river_set_angle (r, .5*(limit[0]+limit[1]));
  return self;
}

AvulsionModel*
avulsion_set_river_hydro (AvulsionModel* self, Sed_hydro hydro)
{
  Sed_riv r = sed_cube_borrow_nth_river (self->p, 0);
  sed_river_set_hydro (r , hydro);
  return self;
}

AvulsionModel*
avulsion_set_river_width (AvulsionModel *self, const double width)
{
  Sed_riv r = sed_cube_borrow_nth_river (self->p, 0);
  sed_river_set_width (r , width);
  return self;
}

AvulsionModel*
avulsion_set_river_depth (AvulsionModel *self, const double depth)
{
  Sed_riv r = sed_cube_borrow_nth_river (self->p, 0);
  sed_river_set_depth (r , depth);
  return self;
}

AvulsionModel*
avulsion_set_river_velocity (AvulsionModel *self, const double velocity)
{
  Sed_riv r = sed_cube_borrow_nth_river (self->p, 0);
  sed_river_set_velocity (r , velocity);
  return self;
}

AvulsionModel*
avulsion_set_river_bed_load_flux (AvulsionModel *self, const double qb)
{
  Sed_riv r = sed_cube_borrow_nth_river (self->p, 0);
  sed_river_set_bedload (r , qb);
  return self;
}

AvulsionModel*
avulsion_set_elevation_from_file (AvulsionModel* self, gchar* file)
{
  eh_require (self);

  {
    gint n_rows = 0;
    gint n_cols = 0;
    GError* error = NULL;
    double** elevation;

    elevation = eh_dlm_read (file, ";,", &n_rows, &n_cols, &error);
    self->elevation = elevation[0];
    g_free (elevation);

    self->nx = n_rows;
    self->ny = n_cols;

    self->discharge = g_new (double, self->nx * self->ny);

    {
      Sed_riv r = sed_river_new ("AvulsionRiver1");
      self->p = sed_cube_new (self->nx, self->ny);
      sed_cube_set_bathy_data (self->p, self->elevation);
      sed_cube_add_trunk (self->p, r);
    }

    if (error)
      self = NULL;
  }

  return self;
}

AvulsionModel*
avulsion_set_sed_flux (AvulsionModel* self, const double flux)
{
  self->sed_flux = flux;
  return self;
}

AvulsionModel*
avulsion_set_discharge (AvulsionModel* self, const double q)
{
  self->init_discharge = q;
  return self;
}

AvulsionModel*
avulsion_set_bed_load_exponent (AvulsionModel* self, double exponent)
{
  self->bed_load_exponent = exponent;
  return self;
}

AvulsionModel*
avulsion_set_discharge_exponent (AvulsionModel* self, double exponent)
{
  self->discharge_exponent = exponent;
  return self;
}

AvulsionModel*
avulsion_set_total_river_mouths (AvulsionModel* self, int n_branches)
{
  if (self->total_river_mouths != n_branches)
  {
    int i;

    self->total_river_mouths = n_branches;
    self->qb = g_renew (double, self->qb, n_branches);
    self->q = g_renew (double, self->q, n_branches);
    self->mouth_x = g_renew (double, self->mouth_x, n_branches);
    self->mouth_y = g_renew (double, self->mouth_y, n_branches);
    self->mouth_qb = g_renew (double, self->mouth_qb, n_branches);
    self->mouth_angle = g_renew (double, self->mouth_angle, n_branches);

    for (i=0; i<n_branches; i++)
    {
      self->qb[i] = 0.;
      self->q[i] = 0.;
      self->mouth_x[i] = 0.;
      self->mouth_y[i] = 0.;
      self->mouth_qb[i] = 0.;
      self->mouth_angle[i] = 0.;
    }

    { /* Split rivers to get the right number of branches */
      const Sed_riv r = sed_cube_borrow_nth_river (self->p, 0);
      //gint n_mouths = sed_cube_n_leaves (self->p);

      //while ((n_branches+1)/2<self->total_river_mouths)
      while (sed_cube_n_leaves (self->p) < self->total_river_mouths)
      {
        sed_cube_split_river (self->p, sed_river_name_loc(r));
        sed_river_impart_avulsion_data(r);

        //n_mouths += 2;
      }
    }

  }
  return self;
}

AvulsionModel*
avulsion_set_elevation (AvulsionModel* self, double* val)
{
  eh_require (self);
  eh_require (val);
  eh_require (self->p);

  {
    const int size = sed_cube_size (self->p);
    int id;
/*
    eh_watch_int (size);
    eh_watch_int (p->nx);
    eh_watch_int (p->ny);
    eh_watch_ptr (p->elevation);

    eh_message ("Init elevation");
    for (i=0; i<len; i++)
      p->elevation[i] = 0.;

    eh_message ("Init val");
    for (i=0; i<len; i++)
      val[i] = 0;
*/
    for (id=0; id<size; id++)
      self->elevation[id] = val[id];

    eh_message ("set bathymetry");
    sed_cube_set_bathy_data (self->p, self->elevation);
    eh_message ("done");
  }

  return self;
}

AvulsionModel*
avulsion_set_depth (AvulsionModel* self, double* val)
{
  {
    const int size = sed_cube_size (self->p);
    int id;

    for (id=0; id<size; id++)
      self->elevation[id] = -1. * val[id];

    sed_cube_set_bathy_data (self->p, self->elevation);
  }
  return self;
}

AvulsionModel*
avulsion_set_value (AvulsionModel* self, const gchar* val_s, double* data,
                    gint lower[3], gint upper[3], gint stride[3])
{
  {
    const int size = sed_cube_size (self->p);
    const int dimen[3] = {avulsion_get_nx (self), avulsion_get_ny (self), 0};
    const int len[3] = {upper[0]-lower[0], upper[1]-lower[1],
                        upper[2]-lower[2]};
    int i, j, k;
    int id;
    int id_0;
    double scale = 1.;

    if (g_ascii_strcasecmp (val_s, "DEPTH")==0)
      scale = -1.;

    for (i=0, k=0; i<dimen[0]; i++)
    {
      id_0 = (i-lower[0])*stride[0];
      for (j=0; j<dimen[1]; j++, k++)
      {
        id = id_0 + (j-lower[1])*stride[1];
        self->elevation[k] = scale * data[id];
        //self->elevation[i][j] = data[id];
      }
    }

    sed_cube_set_bathy_data (self->p, self->elevation);
  }

  return self;
}

double
avulsion_get_variance (AvulsionModel * self)
{
  return self->variance;
}

double
avulsion_get_current_time (AvulsionModel* self)
{
  return self->now;
}

double
avulsion_get_end_time (AvulsionModel* self)
{
  return G_MAXDOUBLE;
}

double
avulsion_get_start_time (AvulsionModel* self)
{
  return 0.;
}

int
avulsion_get_nx (AvulsionModel* self)
{
  return self->nx;
}

int
avulsion_get_ny (AvulsionModel* self)
{
  return self->ny;
}

int*
avulsion_get_value_dimen (AvulsionModel* self, const gchar* val_s, int shape[3])
{
  if (g_ascii_strcasecmp (val_s,
        "channel_outflow_end_bed_load_sediment__mass_flow_rate")==0 ||
      g_ascii_strcasecmp (val_s,
        "channel_outflow_end_water__discharge")==0 ||
      g_ascii_strcasecmp (val_s, "channel_outflow_end__location_model_x_component")==0 ||
      g_ascii_strcasecmp (val_s, "channel_outflow_end__location_model_y_component")==0)
  {
    const int len = self->total_river_mouths;

    shape[0] = len;
    shape[1] = 1;
    shape[2] = 1;
  }
  else
  {
    shape[0] = avulsion_get_ny (self);
    shape[1] = avulsion_get_nx (self);
    shape[2] = 1;
  }

  return shape;
}

double
avulsion_get_dx (AvulsionModel* self)
{
  return self->dx;
}

double
avulsion_get_dy (AvulsionModel* self)
{
  return self->dy;
}

double*
avulsion_get_value_res (AvulsionModel* self, const gchar* val_s, double res[3])
{
  res[0] = avulsion_get_dy (self);
  res[1] = avulsion_get_dx (self);
  res[2] = 1;

  return res;
}

double
avulsion_get_angle (AvulsionModel* self)
{
  return self->angles[self->len-1];
}

int
BMI_AVULSION_Get_output_var_names (void *self, char **names)
{
  AvulsionModel *model = (AvulsionModel*) self;
  int i;

  for (i=0; i<model->output_var_name_count; i++)
    strncpy (names[i], model->output_var_names[i], BMI_AVULSION_VAR_NAME_MAX);
  return BMI_SUCCESS;
}

int
BMI_AVULSION_Get_output_var_name_count (void *self, int *number_of_output_vars)
{
  if (number_of_output_vars) {
    *number_of_output_vars = ((AvulsionModel*)self)->output_var_name_count;
    return BMI_SUCCESS;
  }
  else
    return BMI_FAILURE;
}

int
BMI_AVULSION_Get_input_var_names (void *self, char **names)
{
  AvulsionModel *model = (AvulsionModel*) self;
  int i;

  for (i=0; i<model->input_var_name_count; i++)
    strncpy (names[i], model->input_var_names[i], BMI_AVULSION_VAR_NAME_MAX);
  return BMI_SUCCESS;
}

int
BMI_AVULSION_Get_input_var_name_count (void *self, int *number_of_input_vars)
{
  if (number_of_input_vars) {
    *number_of_input_vars = ((AvulsionModel*)self)->input_var_name_count;
    return BMI_SUCCESS;
  }
  else
    return BMI_FAILURE;
}

int
BMI_AVULSION_Get_var_type (void *self, const char * name, char *type)
{
  if (type) {
    strncpy(type, "double", 2048);
    return BMI_SUCCESS;
  }
  else {
    type[0] = '\0';
    return BMI_FAILURE;
  }
}

int
BMI_AVULSION_Get_var_units (void *self, const char *name, char * units)
{
  int rtn = BMI_FAILURE;

  if (units) {
    int error = BMI_SUCCESS;

    if (strcmp (name, "channel_outflow_end_bed_load_sediment__mass_flow_rate")==0)
      strncpy(units, "kg / s", 2048);
    else if (strcmp (name, "channel_outflow_end_water__discharge")==0)
      strncpy(units, "m^3 / s", 2048);
    else if (strcmp (name, "surface__elevation")==0)
      strncpy(units, "m", 2048);
    else if (strcmp (name, "surface_bed_load_sediment__mass_flow_rate")==0)
      strncpy(units, "kg / s", 2048);
//    else if (strcmp (name, "channel_outflow_end_suspended_sediment__discharge")==0)
//      src = model->discharge;
    else if (strcmp (name, "channel_outflow_end__location_model_x_component")==0)
      strncpy(units, "m", 2048);
    else if (strcmp (name, "channel_outflow_end__location_model_y_component")==0)
      strncpy(units, "m", 2048);
    else if (strcmp (name, "channel_inflow_end_to_channel_outflow_end__angle")==0)
      strncpy(units, "radian", 2048);
    else if (strcmp (name, "avulsion_model__random_walk_variance_constant")==0)
      strncpy(units, "radian", 2048);
    else if (strcmp (name, "avulsion_model__sediment_bed_load_exponent")==0)
      strncpy(units, "-", 2048);
    else if (strcmp (name, "avulsion_model__water_discharge_exponent")==0)
      strncpy(units, "-", 2048);
    else if (strcmp (name, "channel_inflow_end_water__discharge")==0)
      strncpy(units, "m^3 / s", 2048);
    else if (strcmp (name, "channel_inflow_end_bed_load_sediment__mass_flow_rate")==0)
      strncpy(units, "kg / s", 2048);
    else {
      units[0] = '\0';
      error = BMI_FAILURE_BAD_NAME_ERROR;
    }

    if (!error)
      rtn = BMI_SUCCESS;
  }
  else
    rtn = BMI_FAILURE_BAD_ARGUMENT_ERROR;

  return rtn;
}

int
BMI_AVULSION_Get_var_point_count (void *self, const char * name, int *count)
{
  AvulsionModel * model = (AvulsionModel*)self;
  if (count) {
    int rank;
    int error = BMI_AVULSION_Get_var_rank (self, name, &rank);

    if (!error) {
      if (rank == 0)
        *count = 1;
      else if (rank == 1)
        *count = model->total_river_mouths;
      else if (rank == 2)
        *count = avulsion_get_nx (model) * avulsion_get_ny (model);
    }
    else
      return error;
  }
  else
    return BMI_FAILURE;

  return BMI_SUCCESS;
}

int
BMI_AVULSION_Get_var_stride (void *self, const char * name, int *stride)
{
  if (stride) {
    int rank;
    int error = BMI_AVULSION_Get_var_rank (self, name, &rank);

    if (!error) {
      if (rank == 0 || rank == 1)
        stride[0] = 1;
      else if (rank == 2) {
        stride[0] = avulsion_get_ny ((AvulsionModel*)self);
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
BMI_AVULSION_Get_var_size (void *self, const char * name, int *size)
{
  AvulsionModel * model = (AvulsionModel*)self;

  if (size) {
    int rank;
    int error = BMI_AVULSION_Get_var_rank (self, name, &rank);

    if (!error) {
      if (rank == 0)
        *size = 1;
      else if (rank == 1)
        *size = model->total_river_mouths;
      else if (rank == 2)
        *size = avulsion_get_nx (model) * avulsion_get_ny (model);
    }
    else
      return error;
  }
  else
    return BMI_FAILURE;

  return BMI_SUCCESS;
}

int
BMI_AVULSION_Get_var_nbytes (void *self, const char * name, int *nbytes)
{
  int status = BMI_FAILURE;

  {
    int size = 0;
    BMI_AVULSION_Get_var_size (self, name, &size);
    if (status == BMI_FAILURE)
      return status;
    *nbytes = sizeof(double) * size;
    status = BMI_SUCCESS;
  }

  return status;
}

int
BMI_AVULSION_Get_grid_type (void *self, const char * name, char *type)
{
  if (*type) {
    strncpy(type, "uniform_rectilinear", 2048);
    return BMI_SUCCESS;
  } else {
    type[0] = '\0';
    return BMI_FAILURE;
  }
}

int
BMI_AVULSION_Get_grid_shape (void *self, const char * name, int *shape)
{
  AvulsionModel *model = (AvulsionModel*)self;

  if (shape) {
    int rank;
    int error = BMI_AVULSION_Get_var_rank (self, name, &rank);

    if (!error) {
      if (rank == 0)
        shape[0] = 1;
      else if (rank == 1)
        shape[0] = model->total_river_mouths;
      else if (rank == 2) {
        shape[0] = avulsion_get_nx (model);
        shape[1] = avulsion_get_ny (model);
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
BMI_AVULSION_Get_grid_spacing (void *self, const char * name, double *spacing)
{
  AvulsionModel *model = (AvulsionModel*)self;

  if (spacing) {
    int rank;
    int error = BMI_AVULSION_Get_var_rank (self, name, &rank);

    if (!error) {
      if (rank == 0 || rank == 1)
        spacing[0] = 0.;
      else if (rank == 2) {
        spacing[0] = avulsion_get_dx (model);
        spacing[1] = avulsion_get_dy (model);
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
BMI_AVULSION_Get_grid_origin (void *self, const char * name, double *origin)
{
  if (origin) {
    int rank;
    int error = BMI_AVULSION_Get_var_rank (self, name, &rank);

    if (!error) {
      if (rank == 0 || rank == 1)
        origin[0] = 0.;
      else if (rank == 2) {
        origin[0] = 0.;
        origin[1] = 0.;
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
BMI_AVULSION_Get_value_ptr (void *self, const char *name, void **dest)
{
  AvulsionModel *model = (AvulsionModel*)self;
  int rtn = BMI_FAILURE;

  if (dest) {
    double *src = NULL;
    int error = BMI_SUCCESS;

    if (!has_output_var (model, name))
      return BMI_FAILURE_BAD_NAME_ERROR;

    if (strcmp (name, "channel_outflow_end_bed_load_sediment__mass_flow_rate")==0)
      src = model->mouth_qb;
    else if (strcmp (name, "channel_outflow_end_water__discharge")==0)
      src = model->q;
    else if (strcmp (name, "surface__elevation")==0)
      src = model->elevation;
    else if (strcmp (name, "surface_bed_load_sediment__mass_flow_rate")==0)
      src = model->discharge;
//    else if (strcmp (name, "channel_outflow_end_suspended_sediment__discharge")==0)
//      src = model->discharge;
    else if (strcmp (name, "channel_outflow_end__location_model_x_component")==0)
      src = model->mouth_x;
    else if (strcmp (name, "channel_outflow_end__location_model_y_component")==0)
      src = model->mouth_y;
    else if (strcmp (name, "channel_inflow_end_to_channel_outflow_end__angle")==0)
      src = model->mouth_angle;
    else if (strcmp (name, "avulsion_model__random_walk_variance_constant")==0)
      src = &model->variance;
    else if (strcmp (name, "avulsion_model__sediment_bed_load_exponent")==0)
      src = &model->bed_load_exponent;
    else if (strcmp (name, "avulsion_model__water_discharge_exponent")==0)
      src = &model->discharge_exponent;
    else if (strcmp (name, "channel_inflow_end_water__discharge")==0)
      src = &model->init_discharge;
    else if (strcmp (name, "channel_inflow_end_bed_load_sediment__mass_flow_rate")==0)
      src = &model->sed_flux;
    else
      error = BMI_FAILURE_BAD_NAME_ERROR;

    if (!error) {
      rtn = BMI_SUCCESS;
      *dest = src;
    }
  }
  else
    rtn = BMI_FAILURE_BAD_ARGUMENT_ERROR;

  return rtn;
}

int
BMI_AVULSION_Get_value (void *self, const char *name, void *dest)
{
  int rtn = BMI_FAILURE;

  if (dest) {
    void *src = NULL;
    int err = BMI_AVULSION_Get_value_ptr (self, name, &src);

    if (!err && src) { /* Copy the data */
      int nbytes;
      BMI_AVULSION_Get_var_nbytes (self, name, &nbytes);
      memcpy (dest, src, nbytes);

      rtn = BMI_SUCCESS;
    }
    else
      rtn = err;
  }
  else
    rtn = BMI_FAILURE_BAD_ARGUMENT_ERROR;
  return rtn;
}

int
BMI_AVULSION_Get_value_at_indices (void *self, const char *name, void *dest,
    int * inds, int len)
{
  int status = BMI_FAILURE;

  {
    char *src = NULL;
    const int itemsize = sizeof(double);

    status = BMI_AVULSION_Get_value_ptr (self, name, (void**)&src);
    if (status == BMI_FAILURE)
      return status;

    { /* Copy the data */
      int i;
      int offset;
      char * ptr;
      for (i=0, ptr=(char*)dest; i<len; i++, ptr+=itemsize) {
        offset = inds[i] * itemsize;
        memcpy (ptr, src + offset, itemsize);
      }
    }
  }

  return BMI_SUCCESS;
}

int
BMI_AVULSION_Set_value (void *self, const char *name, void *src)
{
  AvulsionModel *model = (AvulsionModel*)self;

  int rtn = BMI_FAILURE;
  if (src) {
    void *dest = NULL;
    int err;

    if (!has_input_var (model, name))
      return BMI_FAILURE_BAD_NAME_ERROR;

    err = BMI_AVULSION_Get_value_ptr (self, name, &dest);

    if (!err) { /* Copy the data */
      int nbytes;
      BMI_AVULSION_Get_var_nbytes (self, name, &nbytes);
      memcpy (dest, src, nbytes);

      if (strcmp (name, "surface__elevation") == 0) {
        eh_message ("set bathymetry");
        sed_cube_set_bathy_data (model->p, model->elevation);
        eh_message ("done");
      }
      rtn = BMI_SUCCESS;
    }
    else
      rtn = err;
  }
  return rtn;
}

int
BMI_AVULSION_Set_value_at_indices (void *self, const char *name, int * inds,
    int len, void *src)
{
  int status = BMI_FAILURE;

  {
    char *dest = NULL;
    const int itemsize = sizeof(double);

    status = BMI_AVULSION_Get_value_ptr (self, name, (void**)&dest);
    if (status == BMI_FAILURE)
      return status;

    { /* Copy the data */
      int i;
      int offset;
      char * ptr;
      for (i=0, ptr=(char*)src; i<len; i++, ptr+=itemsize) {
        offset = inds[i] * itemsize;
        memcpy (dest + offset, ptr, itemsize);
      }

      if (strcmp (name, "surface__elevation") == 0) {
        AvulsionModel * model = (AvulsionModel*)self;
        sed_cube_set_bathy_data (model->p, model->elevation);
      }
    }
  }

  return BMI_SUCCESS;
}

const double*
avulsion_get_value (AvulsionModel* self, const gchar* val_string,
                    gint dimen[3])
{
  double* vals = NULL;
  
  {
    double* src = NULL;

    if (g_ascii_strcasecmp (val_string,
                            "channel_outflow_end_bed_load_sediment__mass_flow_rate")==0)
      src = self->mouth_qb;
    else if (g_ascii_strcasecmp (val_string,
                                "channel_outflow_end_water__discharge")==0)
      src = self->q;
    else if (g_ascii_strcasecmp (val_string, "surface__elevation")==0)
      src = self->elevation;
    else if (g_ascii_strcasecmp (val_string, "surface_water__discharge")==0 ||
             g_ascii_strcasecmp (val_string, "channel_outflow_end_suspended_sediment__discharge")==0)
      src = self->discharge;
    else if (g_ascii_strcasecmp (val_string, "channel_outflow_end__location_model_x_component")==0)
      src = self->mouth_x;
    else if (g_ascii_strcasecmp (val_string, "channel_outflow_end__location_model_y_component")==0)
      src = self->mouth_y;
    else if (g_ascii_strcasecmp (val_string, "channel_inflow_end_to_channel_outflow_end__angle")==0)
      src = self->mouth_angle;

    if (src)
    {
      dimen[0] = 1;
      if (g_ascii_strcasecmp (val_string,
                              "channel_outflow_end_bed_load_sediment__mass_flow_rate")==0 ||
          g_ascii_strcasecmp (val_string,
                              "channel_outflow_end_water__discharge")==0 ||
          g_ascii_strcasecmp (val_string, "channel_outflow_end__location_model_x_component")==0 ||
          g_ascii_strcasecmp (val_string, "channel_outflow_end__location_model_y_component")==0)
      {
        //const int len = avulsion_get_nx (self)*avulsion_get_ny (self);
        const int len = self->total_river_mouths;
        //vals = (double*)g_memdup (src, sizeof (double)*len);
        vals = src;

        dimen[0] = len;
        dimen[1] = 1;
        dimen[2] = 1;
      }
      else
      {
        const int len = avulsion_get_nx (self)*avulsion_get_ny (self);
        //vals = (double*)g_memdup (src, sizeof (double)*len);
        vals = src;

        dimen[1] = avulsion_get_nx (self);
        dimen[2] = avulsion_get_ny (self);
      }
    }
    else
    {
      dimen[0] = 0;
      dimen[1] = 0;
      dimen[2] = 0;
    }
  }

  return (const double*)vals;
}

const double*
avulsion_get_value_data (AvulsionModel* self, const gchar* val_string,
                         gint lower[3], gint upper[3], gint stride[3])
{
  double* val = NULL;
  gint dimen[3];

  val = (double*)avulsion_get_value (self, val_string, dimen);

  if (g_ascii_strcasecmp (val_string,
                          "channel_outflow_end_bed_load_sediment__mass_flow_rate")==0 ||
      g_ascii_strcasecmp (val_string,
                          "channel_outflow_end_water__discharge")==0 ||
      g_ascii_strcasecmp (val_string, "channel_outflow_end__location_model_x_component")==0 ||
      g_ascii_strcasecmp (val_string, "channel_outflow_end__location_model_y_component")==0)
  {
    lower[0] = 0;
    upper[0] = self->total_river_mouths - 1;
    stride[0] = 1;

    lower[1] = 0;
    upper[1] = 0;
    stride[1] = 0;
  }
  else
  {
    lower[0] = 0;
    lower[1] = 0;
    upper[0] = sed_cube_n_y (self->p) - 1;
    upper[1] = sed_cube_n_x (self->p) - 1;
    stride[0] = 1;
    stride[1] = sed_cube_n_y (self->p);
  }

  return (const double*)val;
}

int
_avulsion_initialize (AvulsionModel* s)
{
  if (s)
    return TRUE;
  else
    return FALSE;
}

double*
_split_discharge (double* slope, const int len, const double n,
                  const double q_total, double* mem)
{
  double* q = NULL;

  {
    int i;
    double normalize;

    if (mem)
      q = mem;
    else
      q = g_new (double, len );

    for (i=0, normalize=0.; i<len; i++)
      normalize += pow (slope[i], n);
    normalize = q_total / normalize;
/*
fprintf (stderr, "** q[0] = %f\n", q[0]);
fprintf (stderr, "** slope[0] = %f\n", slope[0]);
fprintf (stderr, "** n = %f\n", n);
fprintf (stderr, "** q_total = %f\n", q_total);
*/
    for (i=0; i<len; i++)
      q[i] = pow (slope[i], n) * normalize;
//fprintf (stderr, "** q[0] = %f\n", q[0]);
  }

  return q;
}

double*
_split_bed_load (double* slope, double* q, const int len, const double m,
                 const double qb_total, double* mem)
{
  double* qb = NULL;

  {
    int i;
    double total = 0;

    if (mem)
      qb = mem;
    else
      qb = g_new (double, len);
    /*
fprintf (stderr, "Split bed load\n");
fprintf (stderr, "len=%d\n", len);
fprintf (stderr, "q=%f\n", q[0]);
fprintf (stderr, "qb=%f\n", qb[0]);
*/
    for (i=0; i<len; i++)
      qb[i] = pow (q[i]*slope[i], m);
//fprintf (stderr, "qb=%f\n", qb[0]);

    for (i=0; i<len; i++)
      total += qb[i];
//fprintf (stderr, "total=%f\n", total);

    if (total > 0.) {
      total = qb_total / total;
      for (i=0; i<len; i++)
        qb[i] *= total;
    }
//fprintf (stderr, "qb=%f\n", qb[0]);
  }

  return qb;
}

double*
_avulsion_branch_length (AvulsionModel* s, int* len)
{
  double* l = NULL;

  *len = 0;

  { /* Calculate leaf lengths, and total length */
    int n;
    Eh_ind_2 hinge;
    Eh_ind_2 mouth;
    const double dx = sed_cube_x_res (s->p);
    const double dy = sed_cube_y_res (s->p);
    double di, dj;
    const Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
    Sed_riv* leaves = sed_river_leaves (r);
    Sed_riv* leaf;
    const int n_leaves = g_strv_length ((gchar**)leaves);

    l = g_new (double, n_leaves);

    for (leaf=leaves, n=0; *leaf; leaf++, n++)
    {
      hinge = sed_river_hinge (*leaf);
      mouth = sed_river_mouth (*leaf);
      di = (hinge.i - mouth.i)*dy;
      dj = (hinge.j - mouth.j)*dx;

      l[n] = sqrt (di*di + dj*dj);

      //fprintf (stderr, "hinge = %d, %d\n", hinge.i, hinge.j);
      //fprintf (stderr, "mouth = %d, %d\n", mouth.i, mouth.j);
    }

    *len = n_leaves;

    eh_free (leaves);
  }

  return l;
}
    
/** The discharge to each branch
*/
double*
_avulsion_branch_discharge (AvulsionModel* s, int* len)
{
  double* q = NULL;

  *len = 0;

  {
    int i;
    const Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
    const Sed_riv* leaves = sed_river_leaves (r);
    //const double q_total = sed_river_water_flux (r);
    //const double q_total = 10000;
    const double q_total = s->init_discharge;
    const double n = s->discharge_exponent;
    const int n_leaves = g_strv_length ((gchar**)leaves);
    double* length = NULL;
    double* slope = NULL;

    slope = g_new (double, n_leaves);

    length = _avulsion_branch_length (s, len);
    eh_require (length);
    eh_require (*len==n_leaves);

    for (i=0; i<n_leaves; i++)
      slope[i] = 1./length[i];

    q = _split_discharge (slope, n_leaves, n, q_total, s->q);

    g_free  (length);
    g_free  (slope);

    eh_free (leaves);

    *len = n_leaves;
  }

  return q;
}

/** The bed load flux to each branch
*/
double*
_avulsion_branch_bed_load (AvulsionModel* s, int* len)
{
  double* qb = NULL;

  *len = 0;

  {
    int i;
    const Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
    //const double qb_total = sed_river_bedload (r);
    const double qb_total = s->sed_flux;
    const Sed_riv* leaves = sed_river_leaves (r);
    const double m = s->bed_load_exponent;
    const int n_leaves = g_strv_length ((gchar**)leaves);
    double* length = NULL;
    double* slope = NULL;
    //double* q = NULL;

    slope = g_new (double, n_leaves);

    length = _avulsion_branch_length (s, len);

    for (i=0; i<n_leaves; i++)
      slope[i] = 1./length[i];

    //q = _avulsion_branch_discharge (s, len);
    _avulsion_branch_discharge (s, len);

    eh_require (*len==n_leaves);
    qb = _split_bed_load (slope, s->q, n_leaves, m, qb_total, s->qb);

    { /* Print leaf info */
      double* angle = g_new (double, n_leaves);

      for (i=0; i<n_leaves; i++)
      {
        angle[i] = sed_river_angle (leaves[i]);
      }
/*
      for (i=0; i<n_leaves; i++)
      {
        fprintf (stderr, "Avulsion: %d: %f: %f: %f: %f: %f: %f: %f\n",
                          i, length[i], s->q[i], s->qb[i], angle[i],
                          s->mouth_x[i], s->mouth_y[i], s->mouth_qb[i]);
      }
*/

      g_free (angle);
    }

    //g_free (q);
    g_free (length);
    g_free (slope);

    eh_free (leaves);
  }

  return qb;
}

#if 0
double*
avulsion_branch_load_fraction (AvulsionModel* s, Sed_riv* leaves, int* len)
{
  double* f = NULL;
  
  *len = 0;

  { /* Calculate fraction to each branch */
    const int n_leaves = g_strv_length ((gchar**)leaves);
    double* lengths = g_new (double, n_leaves);

    f = g_new (double, n_leaves);

    { /* Calculate leaf lengths, and total length */
      int n;
      Eh_ind_2 hinge;
      Eh_ind_2 mouth;
      const double dx = sed_cube_x_res (s->p);
      const double dy = sed_cube_y_res (s->p);
      double di, dj;
      Sed_riv* leaf;

      for (leaf=leaves, n=0; *leaf; leaf++, n++)
      {
        hinge = sed_river_hinge (*leaf);
        mouth = sed_river_mouth (*leaf);
        di = (hinge.i - mouth.i)*dy;
        dj = (hinge.j - mouth.j)*dx;

        lengths[n] = sqrt (di*di + dj*dj);
      }
    }
    
    { /* Calculate fraction to each branch */
      int n;
      double total = 0;
      const double a = s->bed_load_exponent;
      double total_length = 0;

      for (n=0; n<n_leaves; n++)
        total_length += lengths[n];

      for (n=0; n<n_leaves; n++)
      {
        f[n] = pow ((total_length/lengths[n]), a);
        total += f[n];
      }
      for (n=0; n<n_leaves; n++)
        f[n] /= total;
    }

    g_free (lengths);

    *len = n_leaves;
  }

  return f;
}
#endif

void
_avulsion_reset_discharge (AvulsionModel* s)
{
  {
    int i;

    { /* Reset discharge grid */
      const int len = sed_cube_size (s->p);
      for (i=0; i<len; i++)
        s->discharge[i] = 0.;
    }

    { /* Reset qb for each river mouth */
      const int len = s->total_river_mouths;
      for (i=0; i<len; i++)
        s->mouth_qb[i] = 0.;
    }
  }

  return;
}

void
_avulsion_update_discharge (AvulsionModel* s, const double f)
{
  const Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
  gint* path;
  gint* id;
  Sed_riv* leaves;
  int n_leaves;
  int n;
  Eh_ind_2 mouth_ind;
  //double* qb = NULL;
  const double dx = sed_cube_x_res (s->p);
  const double dy = sed_cube_y_res (s->p);
  double angle;

  leaves = sed_river_leaves (r);

  _avulsion_branch_bed_load (s, &n_leaves);

  //f = avulsion_branch_load_fraction (s, leaves, &n_leaves);

  for (n=0; n<n_leaves; n++)
  {
  //path = sed_cube_river_path_id (s->p, r, TRUE);
    path = sed_cube_river_path_id (s->p, leaves[n], FALSE);

//  fprintf (stderr, "Avulsion:\n");
//  fprintf (stderr, "  angle = %f\n", s->angles[s->len-1]);
/*
  for (id=path; *id>=0; id++)
  {
    //s->discharge[*id] = p->sed_flux;
//    fprintf (stderr, "  river index = %d\n", *id);
//    eh_watch_int (*id);
//    eh_watch_dbl (p->elevation[*id]);
//    eh_watch_dbl (sed_cube_elevation (p->p, 0, *id));
  }
*/

    //s->discharge[*path] += p->sed_flux*f[n];

    s->discharge[*path] += s->qb[n]*f;

    mouth_ind = sed_river_mouth (leaves[n]);

    s->mouth_x[n] = mouth_ind.i*dx;
    s->mouth_y[n] = mouth_ind.j*dy;
    s->mouth_qb[n] += s->qb[n]*f;
    s->mouth_angle[n] = sed_river_angle (leaves[n]);

    //angle = sed_river_angle (leaves[n]);
/*
    fprintf (stderr, "Avulsion: %d: %f: %f: %f: %f: %f: %f\n",
                     n, s->q[n], s->qb[n], s->mouth_angle[n]*180./M_PI,
                     s->mouth_x[n], s->mouth_y[n], s->mouth_qb[n]);
*/

  }

  //g_free (qb);

  eh_free (leaves);
  //eh_free (f);

//  eh_watch_int (*path);
//  eh_watch_dbl (s->discharge[*path]);

  return;
}

int
_avulsion_run_until (AvulsionModel* s, int until)
{
  int status = FALSE;

  g_assert (s);
  if (s)
  {
    const gint len = until - s->now;

    if (len>0)
    {
      gint i;
      const Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
      double f = 10e6;
      double area = sed_cube_area_above (s->p, sed_cube_sea_level(s->p));  
      gint n_branches = sed_cube_n_branches (s->p);
      const double dt_frac = 1./(double)len;

      if (s->angles)
        s->angles = g_renew (double, s->angles, len);
      else
        s->angles = g_new (double, len);

      _avulsion_reset_discharge (s);

      s->len = len;
      for (i=0; i<len; i++)
      {
        if (FALSE)
        {
          while ( area/n_branches > f )
          {
            sed_cube_split_river (s->p, sed_river_name_loc(r));
            sed_river_impart_avulsion_data(r);
            n_branches += 2;
          }
        }

        if ((n_branches+1)/2<s->total_river_mouths)
        {
          sed_cube_split_river (s->p, sed_river_name_loc(r));
          sed_river_impart_avulsion_data(r);
          n_branches += 2;
        }

        sed_cube_avulse_river (s->p , r);
        s->angles[i] = sed_river_angle (r);
        s->last_angle = s->angles[i];

        _avulsion_update_discharge (s, dt_frac);

        //s->angles[i] = avulsion (s->rand, s->last_angle, s->variance);
      }
      s->now = until;
      status = TRUE;
    }
    else
      status = FALSE;

    //_avulsion_update_discharge (s);
  }
  else
    status = FALSE;

  return status;
}

int
_avulsion_finalize (AvulsionModel* s)
{
  return TRUE;
}

#define DEFAULT_VARIANCE (1.)
#define DEFAULT_LAST_ANGLE (0.)
#define DEFAULT_SEED (1945)

void
avulsion_init_state (AvulsionModel* s)
{
  g_assert (s);

  if (s)
  {
    s->variance = DEFAULT_VARIANCE;
    s->last_angle = DEFAULT_LAST_ANGLE;
    s->sed_flux = 0;
    s->total_river_mouths = 1;
    s->bed_load_exponent = 5.;

    s->qb = g_new (double, s->total_river_mouths);
    s->q = g_new (double, s->total_river_mouths);
    s->mouth_x = g_new (double, s->total_river_mouths);
    s->mouth_y = g_new (double, s->total_river_mouths);
    s->mouth_qb = g_new (double, s->total_river_mouths);
    s->mouth_angle = g_new (double, s->total_river_mouths);

    s->now = 0;
    s->time_step = 1.;

    s->seed = DEFAULT_SEED;
    s->rand = g_rand_new_with_seed (s->seed);

    s->p = NULL;

    s->elevation = NULL;
    s->discharge = NULL;
    s->nx = 0;
    s->ny = 0;
    s->dx = 1.;
    s->dy = 1.;

    s->angles = NULL;
    s->len = 0;
  }

  return;
}

void
avulsion_free_state (AvulsionModel* s)
{
  if (s)
  {
    g_rand_free (s->rand);
    s->rand = NULL;

    if (s->discharge)
    {
      g_free (s->discharge);
    }
    s->discharge = NULL;
    if (s->elevation)
    {
      g_free (s->elevation);
    }
    s->elevation = NULL;
    s->nx = 0;
    s->ny = 0;

    g_free (s->angles);
    s->angles = NULL;
    s->len = 0;

    g_free (s->q);
    s->q = NULL;

    g_free (s->qb);
    s->qb = NULL;

    g_free (s->mouth_x);
    s->mouth_x = NULL;

    g_free (s->mouth_y);
    s->mouth_y = NULL;

    g_free (s->mouth_qb);
    s->mouth_qb = NULL;

    g_free (s->mouth_angle);
    s->mouth_angle = NULL;

    s->total_river_mouths = 0;
  }
  return;
}
