#ifndef BMI_SEDFLUX3D_INCLUDED
#define BMI_SEDFLUX3D_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

#include "bmi_sedflux.h"

const int BMI_SEDFLUX3D_COMPONENT_NAME_MAX = 2048;
const int BMI_SEDFLUX3D_VAR_NAME_MAX = 2048;
const int BMI_SEDFLUX3D_UNITS_NAME_MAX = 2048;

int BMI_SEDFLUX3D_Initialize (const char *, BMI_Model **);
int BMI_SEDFLUX3D_Update (BMI_Model *);
int BMI_SEDFLUX3D_Update_until (BMI_Model *, double);
int BMI_SEDFLUX3D_Finalize (BMI_Model *);
int BMI_SEDFLUX3D_Run_model (BMI_Model *);

int BMI_SEDFLUX3D_Get_var_point_count (const BMI_Model * state, const char * name, int * count);
int BMI_SEDFLUX3D_Get_var_type (const BMI_Model *, const char *, BMI_Grid_type *);
int BMI_SEDFLUX3D_Get_var_units (const BMI_Model *, const char *, char *);
int BMI_SEDFLUX3D_Get_var_rank (const BMI_Model *, const char *, int *);
int BMI_SEDFLUX3D_Get_var_stride (const BMI_Model *state, const char* val_s, int *stride);
//const char *BMI_Get_var_name (void *, const char *);

// Assumes arrays start at 0, and have contiguous elements (unit stride).
int BMI_SEDFLUX3D_Get_double (const BMI_Model *, const char *, double *);
int BMI_SEDFLUX3D_Get_double_ptr (const BMI_Model *state, const char* val_s, double **dest);
int BMI_SEDFLUX3D_Get_value (const BMI_Model *, const char *, void *);

int BMI_SEDFLUX3D_Set_double (BMI_Model *, const char *, const double *);
int BMI_SEDFLUX3D_Set_value (BMI_Model *, const char *, const void *);

int BMI_SEDFLUX3D_Get_component_name (const BMI_Model *, char *);

int BMI_SEDFLUX3D_Get_input_var_name_count (const BMI_Model * state, int *count);
int BMI_SEDFLUX3D_Get_input_var_names (const BMI_Model *, char **);
int BMI_SEDFLUX3D_Get_output_var_name_count (const BMI_Model * state, int *count);
int BMI_SEDFLUX3D_Get_output_var_names (const BMI_Model *, char **);

int BMI_SEDFLUX3D_Get_grid_shape (const BMI_Model *, const char *, int *);
int BMI_SEDFLUX3D_Get_grid_spacing (const BMI_Model *, const char *, double *);
int BMI_SEDFLUX3D_Get_grid_origin (const BMI_Model *, const char *, double *);

/*
  IElementSet get_Element_Set (void *handle);
  IValueSet get_Value_Set (void *handle, char *long_var_name, ITimeStamp);
*/

// Since these are just wrappers for other BMI functions, I don't
// think they should be included in the interface definition. They
// could be CMI functions.
int BMI_SEDFLUX3D_Is_scalar (void *, char *);
int BMI_SEDFLUX3D_Is_vector (void *, char *);
int BMI_SEDFLUX3D_Is_grid (void *, char *);
int BMI_SEDFLUX3D_Has_var (void *, char *);

// However, something that indicates if the grid is raster, or
// uniform rectilinear would be needed.
int BMI_SEDFLUX3D_Is_raster_grid (void *, const char *);

int BMI_SEDFLUX3D_Get_current_time (const BMI_Model *, double *);
int BMI_SEDFLUX3D_Get_start_time (const BMI_Model *, double *);
int BMI_SEDFLUX3D_Get_end_time (const BMI_Model *, double *);
int BMI_SEDFLUX3D_Get_time_units (const BMI_Model *, char *);

#define NO_BMI_SEDFLUX3D_GET_GRID_CONNECTIVITY
#define NO_BMI_SEDFLUX3D_GET_GRID_OFFSET
#define NO_BMI_SEDFLUX3D_GET_GRID_X
#define NO_BMI_SEDFLUX3D_GET_GRID_Y
#define NO_BMI_SEDFLUX3D_GET_GRID_Z

#if defined(__cplusplus)
}
#endif

#endif


