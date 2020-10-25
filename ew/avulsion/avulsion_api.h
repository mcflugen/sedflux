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
#include "sed/sed_cube.h"

G_BEGIN_DECLS


#ifndef BMI_SUCCESS
    #define BMI_SUCCESS (0)
#endif

#ifndef BMI_FAILURE
    #define BMI_FAILURE (1)
#endif


const int BMI_AVULSION_COMPONENT_NAME_MAX = 2048;
const int BMI_AVULSION_VAR_NAME_MAX = 2048;
const int BMI_AVULSION_UNITS_NAME_MAX = 2048;

#define BMI_AVULSION_COMPONENT_NAME "Avulsion"

#define NO_BMI_AVULSION_GET_GRID_CONNECTIVITY
#define NO_BMI_AVULSION_GET_GRID_OFFSET
#define NO_BMI_AVULSION_GET_GRID_X
#define NO_BMI_AVULSION_GET_GRID_Y
#define NO_BMI_AVULSION_GET_GRID_Z


typedef struct {
    int input_var_name_count;
    int output_var_name_count;
    const char** input_var_names;
    const char** output_var_names;

    double variance;
    double last_angle;
    double now;
    double time_step;
    double sed_flux;
    double init_discharge;
    int total_river_mouths;
    double bed_load_exponent;
    double discharge_exponent;

    double* qb;
    double* q;
    double* mouth_x;
    double* mouth_y;
    double* mouth_qb;
    double* mouth_angle;

    double* angles;
    int len;

    Sed_cube p;

    double* elevation;
    double* discharge;
    int nx;
    int ny;
    double dx;
    double dy;

    GRand* rand;
    guint seed;
} AvulsionModel;

AvulsionModel*
avulsion_init(AvulsionModel*);
int
avulsion_run_until(AvulsionModel*, double);
AvulsionModel*
avulsion_finalize(AvulsionModel*, int);

AvulsionModel*
avulsion_set_variance(AvulsionModel*, double variance);
AvulsionModel*
avulsion_set_dx(AvulsionModel*, double dx);
AvulsionModel*
avulsion_set_dy(AvulsionModel*, double dy);
AvulsionModel*
avulsion_set_grid(AvulsionModel* self, gint shape[2],
    double res[2]);
AvulsionModel*
avulsion_set_river_hinge(AvulsionModel* self, gint ind[2]);
AvulsionModel*
avulsion_set_river_angle_limit(AvulsionModel* self,
    double limit[2]);
//AvulsionModel* avulsion_set_river_hydro (AvulsionModel* self,
//                                          Sed_hydro hydro);
//
//
AvulsionModel*
avulsion_set_river_hydro(AvulsionModel* self, Sed_hydro hydro);



AvulsionModel*
avulsion_set_river_width(AvulsionModel*, const double);
AvulsionModel*
avulsion_set_river_depth(AvulsionModel*, const double);
AvulsionModel*
avulsion_set_river_velocity(AvulsionModel*, const double);
AvulsionModel*
avulsion_set_river_bed_load_flux(AvulsionModel*, const double);

AvulsionModel*
avulsion_set_total_river_mouths(AvulsionModel* self, int n_branches);
AvulsionModel*
avulsion_set_elevation_from_file(AvulsionModel* self,
    gchar* file);
AvulsionModel*
avulsion_set_sed_flux(AvulsionModel* self, const double val);
AvulsionModel*
avulsion_set_discharge(AvulsionModel* self, const double q);
AvulsionModel*
avulsion_set_bed_load_exponent(AvulsionModel* self, double exponent);
AvulsionModel*
avulsion_set_discharge_exponent(AvulsionModel* self,
    double exponent);
AvulsionModel*
avulsion_set_elevation(AvulsionModel* self, double* val);
AvulsionModel*
avulsion_set_depth(AvulsionModel* self, double* val);

double
avulsion_get_variance(AvulsionModel*);
double
avulsion_get_current_time(AvulsionModel* self);
double
avulsion_get_end_time(AvulsionModel* self);
double
avulsion_get_start_time(AvulsionModel* self);
int
avulsion_get_nx(AvulsionModel* self);
int
avulsion_get_ny(AvulsionModel* self);
int*
avulsion_get_value_dimen(AvulsionModel* self, const gchar* val_s,
    int dimen[3]);
double
avulsion_get_dx(AvulsionModel* self);
double
avulsion_get_dy(AvulsionModel* self);
double*
avulsion_get_value_res(AvulsionModel* self, const gchar* val_s,
    double res[3]);
double
avulsion_get_angle(AvulsionModel* self);
const double*
avulsion_get_value(AvulsionModel* self, const gchar* val_string,
    gint dimen[3]);
const double*
avulsion_get_value_data(AvulsionModel* self, const gchar* val_string,
    gint lower[3], gint upper[3], gint stride[3]);

// extern int BMI_AVULSION_Initialize (const char *config_file, void **handle);
extern int
BMI_AVULSION_Initialize(AvulsionModel* self, const char* config_file);
extern int
BMI_AVULSION_Finalize(void* self);
extern int
_avulsion_run_until(AvulsionModel* s, int until);
extern double
avulsion_get_current_time(AvulsionModel* self);
extern double
avulsion_get_end_time(AvulsionModel* self);

G_END_DECLS

#endif

