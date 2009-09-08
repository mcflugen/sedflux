#if !defined (SEDFLUX_API_H)
#define SEDFLUX_API_H

typedef struct Sedflux_state Sedflux_state;

gboolean sedflux (const gchar* init_file, int dimen);

Sedflux_state* sedflux_initialize (const char* file, int dimen);
void sedflux_run_time_step (Sedflux_state* state);
void sedflux_run (Sedflux_state* state);
void sedflux_run_until (Sedflux_state* state, double then);
void sedflux_finalize (Sedflux_state* state);

double* sedflux_get_value (Sedflux_state* state, const char* val_s, int dimen[3]);
double* sedflux_get_value_cube (Sedflux_state* state, const char* val_s, int dimen[3]);
double sedflux_get_start_time (Sedflux_state* state);
double sedflux_get_end_time (Sedflux_state* state);
int sedflux_get_nx (Sedflux_state* state);
int sedflux_get_ny (Sedflux_state* state);
double sedflux_get_xres (Sedflux_state* state);
double sedflux_get_yres (Sedflux_state* state);

void sedflux_set_basement (Sedflux_state* state, const double* val);

#endif

