#ifndef BMI_H
#define BMI_H

#if defined(__cplusplus)
extern "C" {
#endif

#define BMI_SUCCESS (0)
#define BMI_FAILURE (1)

#define BMI_MAX_UNITS_NAME (2048)
#define BMI_MAX_TYPE_NAME (2048)
#define BMI_MAX_COMPONENT_NAME (2048)
#define BMI_MAX_VAR_NAME (2048)


typedef struct BMI_Model {
  void *data;

  /* Initialize, run, finalize (IRF) */
  int (*initialize)(struct BMI_Model *self, const char *config_file);
  int (*update)(struct BMI_Model *self);
  int (*update_until)(struct BMI_Model *self, double then);
  int (*finalize)(struct BMI_Model *self);

  /* Exchange items */
  int (*get_component_name)(struct BMI_Model *self, char *name);
  int (*get_input_item_count)(struct BMI_Model *self, int *count);
  int (*get_output_item_count)(struct BMI_Model *self, int *count);
  int (*get_input_var_names)(struct BMI_Model *self, char **names);
  int (*get_output_var_names)(struct BMI_Model *self, char **names);

  /* Variable information */
  int (*get_var_grid)(struct BMI_Model *self, const char *name, int *grid);
  int (*get_var_type)(struct BMI_Model *self, const char *name, char *type);
  int (*get_var_units)(struct BMI_Model *self, const char *name, char *units);
  int (*get_var_itemsize)(struct BMI_Model *self, const char *name, int *size);
  int (*get_var_nbytes)(struct BMI_Model *self, const char *name, int *nbytes);
  int (*get_var_location)(struct BMI_Model *self, const char *name, char *location);

  /* Time information */
  int (*get_current_time)(struct BMI_Model *self, double *time);
  int (*get_start_time)(struct BMI_Model *self, double *time);
  int (*get_end_time)(struct BMI_Model *self, double *time);
  int (*get_time_units)(struct BMI_Model *self, char *units);
  int (*get_time_step)(struct BMI_Model *self, double *time_step);

  /* Getters */
  int (*get_value)(struct BMI_Model *self, const char *name, void *dest);
  int (*get_value_ptr)(struct BMI_Model *self, const char *name, void **dest_ptr);
  int (*get_value_at_indices)(struct BMI_Model *self, const char *name, void *dest, int *inds, int count);

  /* Setters */
  int (*set_value)(struct BMI_Model *self, const char *name, void *src);
  int (*set_value_at_indices)(struct BMI_Model *self, const char *name, int *inds, int count, void *src);

  /* Grid information */
  int (*get_grid_rank)(struct BMI_Model *self, int grid, int *rank);
  int (*get_grid_size)(struct BMI_Model *self, int grid, int *size);
  int (*get_grid_type)(struct BMI_Model *self, int grid, char *type);

  /* Uniform rectilinear */
  int (*get_grid_shape)(struct BMI_Model *self, int grid, int *shape);
  int (*get_grid_spacing)(struct BMI_Model *self, int grid, double *spacing);
  int (*get_grid_origin)(struct BMI_Model *self, int grid, double *origin);

  /* Non-uniform rectilinear, curvilinear */
  int (*get_grid_x)(struct BMI_Model *self, int grid, double *x);
  int (*get_grid_y)(struct BMI_Model *self, int grid, double *y);
  int (*get_grid_z)(struct BMI_Model *self, int grid, double *z);

  /* Unstructured */
  int (*get_grid_node_count)(struct BMI_Model *self, int grid, int *count);
  int (*get_grid_edge_count)(struct BMI_Model *self, int grid, int *count);
  int (*get_grid_face_count)(struct BMI_Model *self, int grid, int *count);
  int (*get_grid_edge_nodes)(struct BMI_Model *self, int grid, int *edge_nodes);
  int (*get_grid_face_edges)(struct BMI_Model *self, int grid, int *face_edges);
  int (*get_grid_face_nodes)(struct BMI_Model *self, int grid, int *face_nodes);
  int (*get_grid_nodes_per_face)(struct BMI_Model *self, int grid, int *nodes_per_face);
} BMI_Model;


#if defined(__cplusplus)
}
#endif

#endif
