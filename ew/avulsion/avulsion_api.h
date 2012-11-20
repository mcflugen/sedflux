#if !defined( AVULSION_API_H )
#define AVULSION_API_H 

#if !defined(TRUE)
#define TRUE (1)
#endif

#if !defined(FALSE)
#define FALSE (0)
#endif

#include <glib.h>
#include "sed/sed_hydro.h"

G_BEGIN_DECLS

/** An opaque data structure that holds the state of an instance of an
avulsion model.
*/
typedef struct _BMI_Model BMI_Model;

#ifndef BMI_SUCCESS
# define BMI_SUCCESS (0)
#endif

#ifndef BMI_FAILURE
# define BMI_FAILURE (1)
#endif

typedef enum {
  BMI_VAR_TYPE_UNKNOWN = 0,
  BMI_VAR_TYPE_CHAR,
  BMI_VAR_TYPE_UNSIGNED_CHAR,
  BMI_VAR_TYPE_INT,
  BMI_VAR_TYPE_LONG,
  BMI_VAR_TYPE_UNSIGNED_INT,
  BMI_VAR_TYPE_UNSIGNED_LONG,
  BMI_VAR_TYPE_FLOAT,
  BMI_VAR_TYPE_DOUBLE,
  BMI_VAR_TYPE_COUNT
}
BMI_Var_type;

typedef enum {
  BMI_GRID_TYPE_UNKNOWN = 0,
  BMI_GRID_TYPE_UNIFORM,
  BMI_GRID_TYPE_RECTILINEAR,
  BMI_GRID_TYPE_STRUCTURED,
  BMI_GRID_TYPE_UNSTRUCTURED,
  BMI_GRID_TYPE_COUNT
}
BMI_Grid_type;

const int BMI_COMPONENT_NAME_MAX = 2048;
const int BMI_VAR_NAME_MAX = 2048;
const int BMI_UNITS_NAME_MAX = 2048;

#define BMI_COMPONENT_NAME "Avulsion"

int BMI_Initialize (const char *config_file, BMI_Model **handle);

int BMI_Get_component_name (BMI_Model *self, char *name);
int BMI_Get_start_time (BMI_Model *self, double *time);
int BMI_Get_current_time (BMI_Model *self, double *time);
int BMI_Get_end_time (BMI_Model *self, double *time);
int BMI_Get_time_units (BMI_Model *self, char *units);

int BMI_Update (BMI_Model *self);
int BMI_Update_until (BMI_Model *self, double time_in_days);
int BMI_Finalize (BMI_Model *self);

int BMI_Get_output_var_names (BMI_Model *self, char **names);
int BMI_Get_output_var_name_count (BMI_Model *self, int *number_of_output_vars);
int BMI_Get_input_var_names (BMI_Model *self, char **names);
int BMI_Get_input_var_name_count (BMI_Model *self, int *number_of_input_vars);

int BMI_Get_var_rank (BMI_Model *self, const char * name, int *rank);
int BMI_Get_var_type (BMI_Model *self, const char * name, BMI_Var_type *type);
int BMI_Get_var_point_count (BMI_Model *self, const char * name, int *count);
int BMI_Get_var_stride (BMI_Model *self, const char * name, int *stride);

int BMI_Get_grid_type (BMI_Model *self, const char * name, BMI_Grid_type *type);
int BMI_Get_grid_shape (BMI_Model *self, const char * name, int *shape);
int BMI_Get_grid_spacing (BMI_Model *self, const char * name, double *spacing);
int BMI_Get_grid_origin (BMI_Model *self, const char * name, double *origin);

int BMI_Get_double_ptr (BMI_Model *self, const char *name, double **dest);
int BMI_Get_double (BMI_Model *self, const char *name, double *dest);
int BMI_Set_double (BMI_Model *self, const char *name, double *src);

#define NO_BMI_GET_GRID_CONNECTIVITY
#define NO_BMI_GET_GRID_OFFSET
#define NO_BMI_GET_GRID_X
#define NO_BMI_GET_GRID_Y
#define NO_BMI_GET_GRID_Z

BMI_Model* avulsion_init     (BMI_Model*);
int avulsion_run_until(BMI_Model*, double);
BMI_Model* avulsion_finalize (BMI_Model*, int);

BMI_Model* avulsion_set_variance (BMI_Model*, double variance);
BMI_Model* avulsion_set_dx (BMI_Model*, double dx);
BMI_Model* avulsion_set_dy (BMI_Model*, double dy);
BMI_Model* avulsion_set_grid (BMI_Model* self, gint shape[2],
                                   double res[2]);
BMI_Model* avulsion_set_river_hinge (BMI_Model* self, gint ind[2]);
BMI_Model* avulsion_set_river_angle_limit (BMI_Model* self,
                                                double limit[2]);
//BMI_Model* avulsion_set_river_hydro (BMI_Model* self,
//                                          Sed_hydro hydro);
//
//
BMI_Model* avulsion_set_river_hydro (BMI_Model* self, Sed_hydro hydro);



BMI_Model* avulsion_set_river_width (BMI_Model *, const double);
BMI_Model* avulsion_set_river_depth (BMI_Model *, const double);
BMI_Model* avulsion_set_river_velocity (BMI_Model *, const double);
BMI_Model* avulsion_set_river_bed_load_flux (BMI_Model *, const double);

BMI_Model* avulsion_set_total_river_mouths (BMI_Model* self, int n_branches);
BMI_Model* avulsion_set_elevation_from_file (BMI_Model* self,
                                                  gchar* file);
BMI_Model* avulsion_set_sed_flux (BMI_Model* self, const double val);
BMI_Model* avulsion_set_discharge (BMI_Model* self, const double q);
BMI_Model* avulsion_set_bed_load_exponent (BMI_Model* self, double exponent);
BMI_Model* avulsion_set_discharge_exponent (BMI_Model* self,
                                                 double exponent);
BMI_Model* avulsion_set_elevation (BMI_Model* self, double* val);
BMI_Model* avulsion_set_depth (BMI_Model* self, double* val);

double avulsion_get_variance (BMI_Model*);
double avulsion_get_current_time (BMI_Model* self);
double avulsion_get_end_time (BMI_Model* self);
double avulsion_get_start_time (BMI_Model* self);
int avulsion_get_nx (BMI_Model* self);
int avulsion_get_ny (BMI_Model* self);
int* avulsion_get_value_dimen (BMI_Model* self, const gchar* val_s,
                               int dimen[3]);
double avulsion_get_dx (BMI_Model* self);
double avulsion_get_dy (BMI_Model* self);
double* avulsion_get_value_res (BMI_Model* self, const gchar* val_s,
                                double res[3]);
double avulsion_get_angle (BMI_Model* self);
const double* avulsion_get_value (BMI_Model* self, const gchar* val_string,
                            gint dimen[3]);
const double* avulsion_get_value_data (BMI_Model* self, const gchar* val_string,
                                 gint lower[3], gint upper[3], gint stride[3]);

G_END_DECLS

#endif

