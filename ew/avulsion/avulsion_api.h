#if !defined( AVULSION_API_H )
#define AVULSION_API_H 

#if !defined(TRUE)
#define TRUE (1)
#endif

#if !defined(FALSE)
#define FALSE (0)
#endif

#include <glib.h>

G_BEGIN_DECLS

/** An opaque data structure that holds the state of an instance of an
avulsion model.
*/
typedef struct
{
}
Avulsion_state;

Avulsion_state* avulsion_init     (Avulsion_state*);
int avulsion_run_until(Avulsion_state*, double);
Avulsion_state* avulsion_finalize (Avulsion_state*, int);

Avulsion_state* avulsion_set_variance (Avulsion_state*, double variance);
Avulsion_state* avulsion_set_dx (Avulsion_state*, double dx);
Avulsion_state* avulsion_set_dy (Avulsion_state*, double dy);
Avulsion_state* avulsion_set_grid (Avulsion_state* self, gint shape[2],
                                   double res[2]);
Avulsion_state* avulsion_set_river_hinge (Avulsion_state* self, gint ind[2]);
Avulsion_state* avulsion_set_river_angle_limit (Avulsion_state* self,
                                                double limit[2]);
//Avulsion_state* avulsion_set_river_hydro (Avulsion_state* self,
//                                          Sed_hydro hydro);
Avulsion_state* avulsion_set_total_river_mouths (Avulsion_state* self, int n_branches);
Avulsion_state* avulsion_set_elevation_from_file (Avulsion_state* self,
                                                  gchar* file);
Avulsion_state* avulsion_set_sed_flux (Avulsion_state* self, const double val);
Avulsion_state* avulsion_set_discharge (Avulsion_state* self, const double q);
Avulsion_state* avulsion_set_bed_load_exponent (Avulsion_state* self, double exponent);
Avulsion_state* avulsion_set_discharge_exponent (Avulsion_state* self,
                                                 double exponent);
Avulsion_state* avulsion_set_elevation (Avulsion_state* self, double* val);
Avulsion_state* avulsion_set_depth (Avulsion_state* self, double* val);

double avulsion_get_variance (Avulsion_state*);
double avulsion_get_current_time (Avulsion_state* self);
double avulsion_get_end_time (Avulsion_state* self);
double avulsion_get_start_time (Avulsion_state* self);
int avulsion_get_nx (Avulsion_state* self);
int avulsion_get_ny (Avulsion_state* self);
int* avulsion_get_value_dimen (Avulsion_state* self, const gchar* val_s,
                               int dimen[3]);
double avulsion_get_dx (Avulsion_state* self);
double avulsion_get_dy (Avulsion_state* self);
double* avulsion_get_value_res (Avulsion_state* self, const gchar* val_s,
                                double res[3]);
double avulsion_get_angle (Avulsion_state* self);
const double* avulsion_get_value (Avulsion_state* self, const gchar* val_string,
                            gint dimen[3]);
const double* avulsion_get_value_data (Avulsion_state* self, const gchar* val_string,
                                 gint lower[3], gint upper[3], gint stride[3]);

G_END_DECLS

#endif

