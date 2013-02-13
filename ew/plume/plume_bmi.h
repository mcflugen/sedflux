#if !defined (PLUME_BMI_H)
#define PLUME_BMI_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _BMI_Model BMI_Model;

#ifndef BMI_PLUME_COMPONENT_NAME_MAX 
# define BMI_PLUME_COMPONENT_NAME_MAX (2048)
#endif

#ifndef BMI_PLUME_VAR_NAME_MAX 
# define BMI_PLUME_VAR_NAME_MAX (2048)
#endif

#ifndef BMI_PLUME_UNIT_NAME_MAX 
# define BMI_PLUME_UNIT_NAME_MAX (2048)
#endif

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

int BMI_PLUME_Initialize (const char * config_file, BMI_Model **handle);
int BMI_PLUME_Update (BMI_Model *self);
int BMI_PLUME_Update_until (BMI_Model *self, double time);
int BMI_PLUME_Finalize (BMI_Model * self);

int BMI_PLUME_Get_component_name (BMI_Model * self, char *name);
int BMI_PLUME_Get_input_var_name_count (BMI_Model * self, int * input_var_name_count);
int BMI_PLUME_Get_input_var_names (BMI_Model * self, char ** names);
int BMI_PLUME_Get_output_var_name_count (BMI_Model * self, int * output_var_name_count);
int BMI_PLUME_Get_output_var_names (BMI_Model * self, char ** names);

int BMI_PLUME_Get_var_type (BMI_Model * self, const char * name, BMI_Var_type *type);
int BMI_PLUME_Get_var_rank (BMI_Model * self, const char * name, int *rank);
int BMI_PLUME_Get_var_stride (BMI_Model *self, const char * name, int *stride);
int BMI_PLUME_Get_var_point_count (BMI_Model * self, const char * name, int *count);
int BMI_PLUME_Get_var_size (BMI_Model * self, const char * name, int *size);

int BMI_PLUME_Get_start_time (BMI_Model * self, double *time);
int BMI_PLUME_Get_current_time (BMI_Model * self, double *time);
int BMI_PLUME_Get_end_time (BMI_Model * self, double *time);
int BMI_PLUME_Get_time_units (BMI_Model * self, char *units);

int BMI_PLUME_Get_value (BMI_Model * self, const char *name, void *dest);

int BMI_PLUME_Get_double (BMI_Model * self, const char *name, double *dest);
int BMI_PLUME_Get_double_ptr (BMI_Model * self, const char *name, double **dest);
int BMI_PLUME_Set_double (BMI_Model * self, const char *name, double *src);

int BMI_PLUME_Get_grid_type (BMI_Model * self, const char * name, BMI_Grid_type *type);
int BMI_PLUME_Get_grid_shape (BMI_Model * self, const char *name, int *shape);
int BMI_PLUME_Get_grid_spacing (BMI_Model * self, const char *name, double *spacing);
int BMI_PLUME_Get_grid_origin (BMI_Model * self, const char *name, double *origin);

#define NO_BMI_PLUME_GET_GRID_CONNECTIVITY
#define NO_BMI_PLUME_GET_GRID_OFFSET
#define NO_BMI_PLUME_GET_GRID_X
#define NO_BMI_PLUME_GET_GRID_Y
#define NO_BMI_PLUME_GET_GRID_Z

#ifdef __cplusplus
}
#endif

#endif /* plume_bmi.h is included */

