#ifndef BMI_SEDFLUX_INCLUDED
#define BMI_SEDFLUX_INCLUDED

#if defined(__cplusplus)
extern "C" {
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

typedef struct _BMI_Model BMI_Model;

#if defined(__cplusplus)
}
#endif

#endif
