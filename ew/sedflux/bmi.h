#ifndef BMI_INCLUDED
#define BMI_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

//#include "sedflux_api.h"

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
  BMI_VAR_TYPE_NUMBER
}
BMI_Var_type;

typedef enum {
  BMI_GRID_TYPE_UNKNOWN = 0,
  BMI_GRID_TYPE_UNIFORM,
  BMI_GRID_TYPE_RECTILINEAR,
  BMI_GRID_TYPE_STRUCTURED,
  BMI_GRID_TYPE_UNSTRUCTURED,
  BMI_GRID_TYPE_NUMBER
}
BMI_Grid_type;

const int BMI_SUCCESS = 0;
const int BMI_FAILURE = 1;

const int BMI_FAILURE_UNKNOWN_ERROR = 1;
const int BMI_FAILURE_SCAN_ERROR = 2;
const int BMI_FAILURE_BAD_FILE = 3;
const int BMI_FAILURE_BAD_ARGUMENT_ERROR = 4;

const int BMI_SEDFLUX2D_COMPONENT_NAME_MAX = 2048;
const int BMI_SEDFLUX2D_VAR_NAME_MAX = 2048;
const int BMI_SEDFLUX2D_UNITS_NAME_MAX = 2048;

typedef struct _BMI_Model BMI_Model;

//typedef Sedflux_state BMI_Model;
/*
typedef struct {
  Sedflux_state state;

  int input_var_name_count;
  int output_var_name_count;
  int input_var_names;
  int output_var_names;
} BMI_Model_Sedflux2D;

typedef struct {
  Sedflux_state state;

  int input_var_name_count;
  int output_var_name_count;
  int input_var_names;
  int output_var_names;
} BMI_Model_Sedflux3D;
*/

int BMI_SEDFLUX2D_Initialize (const char *, BMI_Model **);
int BMI_SEDFLUX2D_Update (BMI_Model *);
int BMI_SEDFLUX2D_Update_until (BMI_Model *, double);
int BMI_SEDFLUX2D_Finalize (BMI_Model *);
int BMI_SEDFLUX2D_Run_model (BMI_Model *);

int BMI_SEDFLUX2D_Get_var_point_count (const BMI_Model * state, const char * name, int * count);
int BMI_SEDFLUX2D_Get_var_type (const BMI_Model *, const char *, BMI_Grid_type *);
int BMI_SEDFLUX2D_Get_var_units (const BMI_Model *, const char *, char *);
int BMI_SEDFLUX2D_Get_var_rank (const BMI_Model *, const char *, int *);
int BMI_SEDFLUX2D_Get_var_stride (const BMI_Model *state, const char* val_s, int *stride);
//const char *BMI_Get_var_name (void *, const char *);

// Assumes arrays start at 0, and have contiguous elements (unit stride).
int BMI_SEDFLUX2D_Get_double (const BMI_Model *, const char *, double *);
int BMI_SEDFLUX2D_Get_double_ptr (const BMI_Model *state, const char* val_s, double **dest);
int BMI_SEDFLUX2D_Get_value (const BMI_Model *, const char *, void *);

int BMI_SEDFLUX2D_Set_double (BMI_Model *, const char *, const double *);
int BMI_SEDFLUX2D_Set_value (BMI_Model *, const char *, const void *);

int BMI_SEDFLUX2D_Get_component_name (const BMI_Model *, char *);

int BMI_SEDFLUX2D_Get_input_var_name_count (const BMI_Model * state, int *count);
int BMI_SEDFLUX2D_Get_input_var_names (const BMI_Model *, char **);
int BMI_SEDFLUX2D_Get_output_var_name_count (const BMI_Model * state, int *count);
int BMI_SEDFLUX2D_Get_output_var_names (const BMI_Model *, char **);

int BMI_SEDFLUX2D_Get_grid_shape (const BMI_Model *, const char *, int *);
int BMI_SEDFLUX2D_Get_grid_spacing (const BMI_Model *, const char *, double *);
int BMI_SEDFLUX2D_Get_grid_origin (const BMI_Model *, const char *, double *);

/*
  IElementSet get_Element_Set (void *handle);
  IValueSet get_Value_Set (void *handle, char *long_var_name, ITimeStamp);
*/

// Since these are just wrappers for other BMI functions, I don't
// think they should be included in the interface definition. They
// could be CMI functions.
int BMI_SEDFLUX2D_Is_scalar (void *, char *);
int BMI_SEDFLUX2D_Is_vector (void *, char *);
int BMI_SEDFLUX2D_Is_grid (void *, char *);
int BMI_SEDFLUX2D_Has_var (void *, char *);

// However, something that indicates if the grid is raster, or
// uniform rectilinear would be needed.
int BMI_SEDFLUX2D_Is_raster_grid (void *, const char *);

int BMI_SEDFLUX2D_Get_current_time (const BMI_Model *, double *);
int BMI_SEDFLUX2D_Get_start_time (const BMI_Model *, double *);
int BMI_SEDFLUX2D_Get_end_time (const BMI_Model *, double *);
int BMI_SEDFLUX2D_Get_time_units (const BMI_Model *, char *);

#define NO_BMI_SEDFLUX2D_GET_GRID_CONNECTIVITY
#define NO_BMI_SEDFLUX2D_GET_GRID_OFFSET
#define NO_BMI_SEDFLUX2D_GET_GRID_X
#define NO_BMI_SEDFLUX2D_GET_GRID_Y
#define NO_BMI_SEDFLUX2D_GET_GRID_Z

#if defined(__cplusplus)
}
#endif

#endif


