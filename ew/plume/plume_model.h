#ifndef PLUME_MODEL_H_INCLUDED
#define PLUME_MODEL_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _PlumeModel PlumeModel;


double plume_get_start_time(PlumeModel * self);
double plume_get_current_time(PlumeModel * self);
double plume_get_end_time(PlumeModel * self);
double * plume_get_deposition_rate_buffer(PlumeModel * self);
void plume_get_grid_shape(PlumeModel * self, int shape[2]);
double * plume_get_velocity_ptr(PlumeModel * self);
double * plume_get_width_ptr(PlumeModel * self);
double * plume_get_depth_ptr(PlumeModel * self);
double * plume_get_bedload_ptr(PlumeModel * self);
double * plume_get_qs_ptr(PlumeModel * self);
void plume_set_velocity(PlumeModel *self, double val);
void plume_set_width(PlumeModel *self, double val);
void plume_set_depth(PlumeModel *self, double val);
void plume_set_bedload(PlumeModel *self, double val);
void plume_set_qs(PlumeModel *self, double val);
void plume_get_grid_spacing(PlumeModel * self, double spacing[2]);
void plume_get_grid_origin(PlumeModel * self, double origin[2]);
int plume_initialize (const char * config_file, PlumeModel **handle);
int plume_update_until (PlumeModel *self, double time);
int plume_finalize (PlumeModel * self);

#if defined(__cplusplus)
}
#endif

#endif
