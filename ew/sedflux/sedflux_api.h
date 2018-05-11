#if !defined (SEDFLUX_API_H)
#define SEDFLUX_API_H

#include <glib.h>

G_BEGIN_DECLS

extern const gint MASK_LAND;
extern const gint MASK_OCEAN;

typedef struct _Sedflux_state Sedflux_state;

//gboolean sedflux (const gchar* init_file, const gchar* prefix, int dimen);
gboolean sedflux (const int argc, const char* argv[]);

gboolean sedflux_is_2d (Sedflux_state* self);

Sedflux_state* sedflux_initialize (const int argc, const char* argv[]);
//Sedflux_state* sedflux_initialize (const gchar* file, const gchar* prefix,
//                                   int dimen);
void sedflux_run_time_step (Sedflux_state* state);
void sedflux_run (Sedflux_state* state);
void sedflux_run_until (Sedflux_state* state, double then);
void sedflux_finalize (Sedflux_state* state);

gchar** sedflux_get_exchange_items (Sedflux_state* state);
gchar* sedflux_get_exchange_item_unit (Sedflux_state* state, const gchar* name);
gchar** sedflux_get_exchange_cube_items (Sedflux_state* state);
double* sedflux_get_value_data (Sedflux_state* state, const char* val_s,
                                int dimen[3]);
double * sedflux_get_surface_value (Sedflux_state* state, const char* val_s, double *dest, gint mask);
double * sedflux_get_sediment_value (Sedflux_state* state, const char* val_s, double *dest);
double* sedflux_get_value (Sedflux_state* state, const char* val_s, int dimen[3]);
double* sedflux_get_double (Sedflux_state* state, const char* val_s,
                            int * n_dim, int **shape);
double* sedflux_get_value_cube (Sedflux_state* state, const char* val_s,
                                int dimen[3]);
int* sedflux_get_value_dimen (Sedflux_state* state, const char* val_s,
                              int dimen[3]);
int* sedflux_get_value_shape (Sedflux_state* state, const char* val_s,
                              int *n_dim);
double* sedflux_get_value_res (Sedflux_state* state, const char* val_s,
                               double res[3]);
double* sedflux_get_value_spacing (Sedflux_state* state, const char* val_s,
    int *n_dim);
double sedflux_get_start_time (Sedflux_state* state);
double sedflux_get_end_time (Sedflux_state* state);
double sedflux_get_current_time (Sedflux_state* state);
double sedflux_get_time_step(Sedflux_state* state);
int sedflux_get_nx (Sedflux_state* state);
int sedflux_get_ny (Sedflux_state* state);
int sedflux_get_nz (Sedflux_state* state);
double sedflux_get_xres (Sedflux_state* state);
double sedflux_get_yres (Sedflux_state* state);
double sedflux_get_zres (Sedflux_state* state);

void sedflux_set_uplift (Sedflux_state* state, const double* val);
void sedflux_set_basement (Sedflux_state* state, const double* val);
void sedflux_set_erosion (Sedflux_state* state, const double* val);
void sedflux_set_deposition (Sedflux_state* state, const double* val);
void sedflux_set_subaerial_deposition (Sedflux_state* state, const double* val);
void sedflux_set_subaerial_deposition_to (Sedflux_state* state, const double* val);
void sedflux_set_discharge (Sedflux_state* state, const double* val);
void sedflux_set_bed_load_flux (Sedflux_state* state, const double* val);

void sedflux_set_channel_bedload(Sedflux_state* state, const double* val);
void sedflux_set_channel_suspended_load(Sedflux_state* state, const double* val);
void sedflux_set_channel_width(Sedflux_state* state, const double* val);
void sedflux_set_channel_depth(Sedflux_state* state, const double* val);
void sedflux_set_channel_velocity(Sedflux_state* state, const double* val);
void sedflux_set_sea_level(Sedflux_state* state, const double* val);

G_END_DECLS

#endif

