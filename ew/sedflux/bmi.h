#ifndef BMI_H_INCLUDED
#define BMI_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

#define BMI_SUCCESS (0)
#define BMI_FAILURE (1)

#define BMI_MAX_UNITS_NAME (2048)
#define BMI_MAX_COMPONENT_NAME (2048)
#define BMI_MAX_VAR_NAME (2048)


typedef struct {
  void * self;

  int (* initialize)(const char *, void**);
  int (* update)(void*);
  int (* update_until)(void *, double);
  int (* update_frac)(void *, double);
  int (* finalize)(void *);
  int (* run_model)(void *);

  int (* get_component_name)(void *, char *);
  int (* get_input_var_name_count)(void *, int *);
  int (* get_output_var_name_count)(void *, int *);
  int (* get_input_var_names)(void *, char **);
  int (* get_output_var_names)(void *, char **);

  int (* get_var_grid)(void *, const char *, int *);
  int (* get_var_type)(void *, const char *, char *);
  int (* get_var_units)(void *, const char *, char *);
  int (* get_var_itemsize)(void *, const char *, int *);
  int (* get_var_nbytes)(void *, const char *, int *);
  int (* get_var_location)(void *, const char *, char *);
  int (* get_current_time)(void *, double *);
  int (* get_start_time)(void *, double *);
  int (* get_end_time)(void *, double *);
  int (* get_time_units)(void *, char *);
  int (* get_time_step)(void *, double *);

  /* Variable getter and setter functions */
  int (* get_value)(void *, const char *, void *);
  int (* get_value_ptr)(void *, const char *, void **);
  int (* get_value_at_indices)(void *, const char *, void *, int *, int);

  int (* set_value)(void *, const char *, void *);
  int (* set_value_ptr)(void *, const char *, void **);
  int (* set_value_at_indices)(void *, const char *, int *, int, void *);

  /* Grid information functions */
  int (* get_grid_rank)(void *, int, int *);
  int (* get_grid_size)(void *, int, int *);
  int (* get_grid_type)(void *, int, char *);
  int (* get_grid_shape)(void *, int, int *);
  int (* get_grid_spacing)(void *, int, double *);
  int (* get_grid_origin)(void *, int, double *);

  int (* get_grid_x)(void *, int, double *);
  int (* get_grid_y)(void *, int, double *);
  int (* get_grid_z)(void *, int, double *);

  int (* get_grid_face_count)(void *, int, int *);
  int (* get_grid_point_count)(void *, int, int *);
  int (* get_grid_vertex_count)(void *, int, int *);

  int (* get_grid_connectivity)(void *, int, int *);
  int (* get_grid_offset)(void *, int, int *);
} BMI_Model;


#if defined(__cplusplus)
}
#endif

#endif
