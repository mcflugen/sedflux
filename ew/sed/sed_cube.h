//---
//
// This file is part of sedflux.
//
// sedflux is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// sedflux is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with sedflux; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---

#if !defined( SED_CUBE_H )
#define SED_CUBE_H

#include <glib.h>

G_BEGIN_DECLS

#include "utils/utils.h"

#include "sed_sediment.h"
#include "sed_cell.h"
#include "sed_column.h"
#include "sed_const.h"
#include "sed_hydro.h"
#include "sed_wave.h"
#include "sed_river.h"

typedef enum {
    SED_CUBE_ERROR_NO_TIME_LABEL,
    SED_CUBE_ERROR_TIME_NOT_MONOTONIC,
    SED_CUBE_ERROR_NOT_TWO_COLUMNS,
    SED_CUBE_ERROR_INSUFFICIENT_DATA,
    SED_CUBE_ERROR_DATA_NOT_MONOTONIC,
    SED_CUBE_ERROR_DATA_BAD_RANGE,

    SED_CUBE_ERROR_TOO_FEW_ROWS,
    SED_CUBE_ERROR_TOO_FEW_COLUMNS,
    SED_CUBE_ERROR_BAD_GRID_DIMENSION,
    SED_CUBE_ERROR_TRUNCATED_FILE
}
Sed_cube_error;

#define VARY_COLS (0)
#define VARY_ROWS (1)

#define SED_CUBE_SUSP_GRID sed_cube_susp_grid_quark()
#define SED_CUBE_ERROR     sed_cube_error_quark()

GQuark
sed_cube_error_quark(void);
GQuark
sed_cube_susp_grid_quark(void);

typedef enum {
    SEDFLUX_MODE_3D,
    SEDFLUX_MODE_2D,
    SEDFLUX_MODE_NOT_SET
}
Sedflux_mode;

void
sed_mode_set(Sedflux_mode mode);
gboolean
sed_mode_is(Sedflux_mode mode);
gboolean
sed_mode_is_2d(void);
gboolean
sed_mode_is_3d(void);

/**
   A structure to describe the hinge point of a stream
*/
typedef struct {
    double angle;      ///< The angle the stream leaves the hinge point from
    int x;             ///< The x-index of the hinge point
    int y;             ///< The y-index of the hinge point
    double min_angle;  ///< The minimum angle the steam can make
    double max_angle;  ///< The maximum angle the stream can make
    double std_dev;    ///< The standard deviation of stream avulsions
}
Sed_hinge_pt;

/**
   Describe a river.
*/
/*
typedef struct
{
   Sed_hydro data;      ///< The hydrological characteristics of the river
   Sed_hinge_pt *hinge; ///< The hinge point of the river
   int x_ind;           ///< The x-index of the river mouth
   int y_ind;           ///< The y-index of the river mouth
   char *river_name;    ///< The name of the river
}
Sed_river;
*/

new_handle(Sed_cube);

typedef double (*Sed_grid_func)(const Sed_cube, gint, gint);
typedef gboolean(*Sed_cube_func)(const Sed_cube, gssize, gssize, gpointer);

#define S_X_FUNC            (&sed_cube_col_x_ij)
#define S_Y_FUNC            (&sed_cube_col_y_ij)
#define S_X_SLOPE_FUNC      (&sed_cube_x_slope)
#define S_Y_SLOPE_FUNC      (&sed_cube_y_slope)
#define S_SLOPE_DIR_FUNC    (&sed_cube_slope_dir)
#define S_THICKNESS_FUNC    (&sed_cube_thickness)
#define S_ELEVATION_FUNC    (&sed_cube_elevation)
#define S_WATER_DEPTH_FUNC  (&sed_cube_water_depth)
#define S_LOAD_FUNC         (&sed_cube_load)

Sed_cube
sed_cube_new(gssize n_x, gssize n_y);
Sed_cube
sed_cube_new_from_file(const gchar* file, const gchar* prefix,
    GError** error);
Sed_cube
sed_cube_new_empty(gssize n_x, gssize n_y);
Sed_cube
sed_cube_free(Sed_cube c, gboolean free_data);
Sed_cube
sed_cube_free_river(Sed_cube p);
Sed_cube
sed_cube_destroy(Sed_cube s);
Sed_cube
sed_cube_copy_pointer_data(Sed_cube dest, const Sed_cube src);
Sed_cube
sed_cube_copy_scalar_data(Sed_cube dest, const Sed_cube src);
Sed_cell_grid
sed_cube_create_in_suspension(Sed_cube s);
Sed_cell_grid
sed_cube_destroy_in_suspension(Sed_cell_grid g);
gchar*
sed_cube_name(const Sed_cube s);
GSList*
sed_cube_storm_list(Sed_cube c);
Sed_cube
sed_cube_set_storm_list(Sed_cube c, GSList* storms);
Sed_cube
sed_cube_adjust_sea_level(Sed_cube s, double dz);
Sed_cube
sed_cube_destroy_storm_list(Sed_cube c);
Sed_cube
sed_cube_set_bathy(Sed_cube c, Eh_dbl_grid g);
Sed_cube
sed_cube_set_bathy_data(Sed_cube c, double* z);
Sed_cube
sed_cube_set_dz(Sed_cube p, double new_dz);
gint
sed_cube_size(const Sed_cube s);
gint
sed_cube_n_x(const Sed_cube s);
gint
sed_cube_n_y(const Sed_cube s);

Sed_column
sed_cube_col(const Sed_cube s, gssize ind);
Sed_column
sed_cube_col_ij(const Sed_cube s, gssize i, gssize j);
Sed_column
sed_cube_col_pos(const Sed_cube s, double x, double y);

double
sed_cube_sea_level(const Sed_cube s);
double*
sed_cube_x(const Sed_cube s, gint* id);
double*
sed_cube_y(const Sed_cube s, gint* id);
double
sed_cube_col_x(const Sed_cube s, gssize id);
double
sed_cube_col_y(const Sed_cube s, gssize id);
double
sed_cube_col_x_ij(const Sed_cube s, gint i, gint j);
double
sed_cube_col_y_ij(const Sed_cube s, gint i, gint j);
double
sed_cube_x_slope(const Sed_cube s, gint i, gint j);
double
sed_cube_y_slope(const Sed_cube s, gint i, gint j);
double
sed_cube_slope(const Sed_cube s, gssize i, gssize j);
double
sed_cube_slope_dir(const Sed_cube s, gint i, gint j);
double
sed_cube_top_height(const Sed_cube p, gssize i, gssize j);
Eh_ind_2
ind2sub(gssize ind, gssize n_x, gssize n_y);
Eh_dbl_grid
sed_cube_grid(const Sed_cube s, Sed_grid_func f, gint* index);
Eh_dbl_grid
sed_cube_slope_dir_grid(const Sed_cube s, gint* index);
Eh_dbl_grid
sed_cube_x_slope_grid(const Sed_cube s, gint* index);
Eh_dbl_grid
sed_cube_y_slope_grid(const Sed_cube s, gint* index);
double
sed_cube_time_step(const Sed_cube s);
double
sed_cube_time_step_in_years(const Sed_cube s);
double
sed_cube_time_step_in_seconds(const Sed_cube s);
double
sed_cube_time_step_in_days(const Sed_cube s);
Sed_constants
sed_cube_constants(const Sed_cube s);
double
sed_cube_x_res(const Sed_cube s);
double
sed_cube_y_res(const Sed_cube s);
double
sed_cube_z_res(const Sed_cube s);
Sed_cell
sed_cube_to_remove(const Sed_cube s);
Sed_cell
sed_cube_to_add(const Sed_cube s);
gssize
sed_get_column_x_index(const Sed_cube c, double x);
gssize
sed_get_column_y_index(const Sed_cube c, double y);
double
sed_cube_wave_height(const Sed_cube s);
double
sed_cube_wave_period(const Sed_cube s);
double
sed_cube_wave_length(const Sed_cube s);
double
sed_cube_storm(const Sed_cube s);
double
sed_cube_quake(const Sed_cube s);
double
sed_cube_age(const Sed_cube s);
double
sed_cube_age_in_years(const Sed_cube s);
double
sed_cube_tidal_period(const Sed_cube s);
Sed_cube
sed_cube_set_tidal_period(Sed_cube s, double new_val);
double
sed_cube_tidal_range(const Sed_cube s);
Sed_cube
sed_cube_set_tidal_range(Sed_cube s, double new_val);
double
sed_cube_water_depth(const Sed_cube p, gint i, gint j);
double
sed_cube_elevation(const Sed_cube p, gint i, gint j);
double
sed_cube_load(const Sed_cube p, gint i, gint j);
double
sed_cube_water_pressure(const Sed_cube p, gssize i, gssize j);
double
sed_cube_thickness(const Sed_cube p, gint i, gint j);
gboolean
sed_cube_col_is_empty(const Sed_cube p, gssize i, gssize j);
double
sed_cube_base_height(const Sed_cube p, gssize i, gssize j);

Sed_cube
sed_cube_dup(Sed_cube c);
Sed_cube
sed_cube_copy(Sed_cube dest, const Sed_cube src);
Sed_cube
sed_cube_copy_cols(const Sed_cube src, gssize* x, gssize* y, double* z, gssize len);
Sed_cube
sed_cube_copy_line(const Sed_cube src, double* x, double* y, double* z, gssize len);
Sed_cube
sed_cube_cols(Sed_cube src, gint* path);
gssize
sed_cube_river_mouth_1d(Sed_cube c);
Sed_cube
sed_cube_remove(Sed_cube dest, Sed_cube src);
Sed_cube
sed_cube_add(Sed_cube dest, const Sed_cube src);

//Sed_profile *sed_get_profile_from_cube( Sed_cube c , GList *path );
double
sed_cube_mass(const Sed_cube p);
double
sed_cube_sediment_mass(const Sed_cube p);
double
sed_cube_mass_in_suspension(const Sed_cube p);
Sed_cube
sed_cube_set_sea_level(Sed_cube s, double new_sea_level);
Sed_cube
sed_cube_set_base_height(Sed_cube s, gssize i, gssize j, double height);
Sed_cube
sed_cube_adjust_base_height(Sed_cube s, gssize i, gssize j, double dz);
void
sed_cube_set_discharge(Sed_cube s, const double* val);

Sed_hydro
sed_cube_external_river(Sed_cube s);
void
sed_cube_set_external_river_bedload(Sed_cube s, const double val);
void
sed_cube_set_external_river_suspended_load(Sed_cube s, const double val);
void
sed_cube_set_external_river_width(Sed_cube s, const double val);
void
sed_cube_set_external_river_depth(Sed_cube s, const double val);
void
sed_cube_set_external_river_velocity(Sed_cube s, const double val);

void
sed_cube_set_bed_load_flux(Sed_cube s, const double* val);
Sed_cube
sed_cube_set_nth_river(Sed_cube s, gssize n, Sed_riv river);
Sed_riv
sed_cube_borrow_nth_river(Sed_cube s, gint n);
Eh_pt_2*
sed_cube_river_mouth_position(Sed_cube s, Sed_riv this_river);
Sed_hydro
sed_cube_river_data(Sed_cube s, GList* this_river);
/*
Sed_cube    sed_cube_set_river_data( Sed_cube s       ,
                              GList *this_river ,
                              Sed_hydro new_data );
*/
Sed_cube
sed_cube_set_river_list(Sed_cube s, GList* river_list) G_GNUC_DEPRECATED;


Sed_cube
sed_cube_split_river(Sed_cube s, const gchar* name);
void
sed_river_attach_susp_grid(Sed_riv r, Sed_cell_grid g);
Sed_cell_grid
sed_river_get_susp_grid(Sed_riv r);
void
sed_river_detach_susp_grid(Sed_riv r);
gpointer
sed_cube_add_trunk(Sed_cube s, Sed_riv new_trunk);
void
sed_cube_remove_trunk(Sed_cube s, gpointer trunk_id);
void
sed_cube_set_river_path_ray(Sed_riv r, const Sed_cube s,
    const gint start[2], double a);
void
sed_cube_set_river_path_ends(Sed_riv r, const Sed_cube s,
    const gint start[2],
    const gint end[2]);
gpointer
sed_cube_add_river_mouth(Sed_cube s, gint id, Sed_hydro hydro);
Sed_cube
sed_cube_remove_river(Sed_cube s, Sed_riv r);
Sed_cube
sed_cube_remove_all_trunks(Sed_cube s);

Sed_cube
sed_cube_set_name(Sed_cube s, char* name);
Sed_cube
sed_cube_set_time_step(Sed_cube s, double time_step_in_years);
Sed_cube
set_cube_set_constants(Sed_cube s, Sed_constants new_c);
Sed_cube
sed_cube_set_x_res(Sed_cube s, double new_x_res);
Sed_cube
sed_cube_set_y_res(Sed_cube s, double new_x_res);
Sed_cube
sed_cube_set_z_res(Sed_cube s, double new_x_res);
Sed_cube
sed_cube_set_wave_height(Sed_cube s, double new_wave_height);
Sed_cube
sed_cube_set_wave_period(Sed_cube s, double new_wave_period);
Sed_cube
sed_cube_set_wave_length(Sed_cube s, double new_wave_length);
Sed_cube
sed_cube_set_storm(Sed_cube s, double new_storm_value);
Sed_cube
sed_cube_set_quake(Sed_cube s, double new_quake_value);
Sed_cube
sed_cube_set_age(Sed_cube s, double new_age);
Sed_cube
sed_cube_adjust_age(Sed_cube s, double dt);
Sed_cube
sed_cube_increment_age(Sed_cube s);

Eh_dbl_grid
sed_cube_water_depth_grid(const Sed_cube s, gint* index);
Eh_dbl_grid
sed_cube_elevation_grid(const Sed_cube s, gint* index);
Eh_dbl_grid
sed_cube_thickness_grid(const Sed_cube s, gint* index);
Eh_dbl_grid
sed_cube_load_grid(const Sed_cube s, gint* index);

gboolean*
sed_cube_shore_mask(const Sed_cube s);
gint*
sed_cube_shore_ids(const Sed_cube s);

Sed_riv
sed_cube_river_by_name(Sed_cube s, const char* name);
Sed_riv
sed_cube_nth_river(Sed_cube s, gint n);
gssize
sed_cube_river_id(Sed_cube s, Sed_riv river);

double
sed_cube_nth_river_angle(Sed_cube s, gint n);
Eh_ind_2
sed_cube_nth_river_mouth(Sed_cube s, gint n);

//Sed_cell_grid sed_cube_in_suspension( Sed_cube s , gssize river_no );
Sed_cell_grid
sed_cube_in_suspension(Sed_cube s, Sed_riv r);

GList*
sed_cube_river_list(Sed_cube s);
Sed_riv*
sed_cube_all_trunks(Sed_cube s);
Sed_riv*
sed_cube_all_branches(Sed_cube s);
Sed_riv*
sed_cube_all_leaves(Sed_cube s);
Sed_riv*
sed_cube_all_rivers(Sed_cube s);

gssize
sed_cube_number_of_rivers(Sed_cube s) G_GNUC_DEPRECATED;
gint
sed_cube_n_branches(Sed_cube s);
gint
sed_cube_n_leaves(Sed_cube s);
gint
sed_cube_n_rivers(Sed_cube s);

Sed_cube
sed_load_cube(FILE* fp);
gssize
sed_cube_column_id(const Sed_cube c, double x, double y);
void
sed_set_shore(Sed_cube s);
GList*
sed_cube_find_shore_line(Sed_cube s, Eh_ind_2* pos);

Sed_riv
sed_cube_find_river_mouth(Sed_cube c, Sed_riv this_river);

GList*
sed_find_next_shore(GList* shore_list,
    Sed_cube s,
    Eh_ind_2* pos,
    GList* ignore_list);
gint*
sed_cube_shore_normal_shift(Sed_cube s, gint i, gint j);
double
sed_cube_shore_normal(Sed_cube s, gint i, gint j);
GList*
sed_find_columns_custom(Sed_cube s,
    gssize i,
    gssize j,
    gpointer data,
    Sed_cube_func stop_func,
    Sed_cube_func angle_func);
GList*
sed_cube_find_cross_shore_columns(Sed_cube s, gssize i, gssize j);
Eh_ind_2*
sed_cube_find_shore(Sed_cube s, int n, int vary_dim);



Eh_ind_2*
sed_find_adjacent_shore_edge(Sed_cube s,
    gssize i,
    gssize j,
    int edge);
int
sed_rotate_direction(int dir, int angle);
gboolean
sed_cube_is_shore_edge(Sed_cube s, gssize i, gssize j, int edge);
Eh_ind_2
sed_shift_index_over_edge(gssize i, gssize j, int edge);



GTree*
sed_create_shore_tree(GList* shore);
gint*
sed_cube_river_path_id(Sed_cube c, Sed_riv river, gboolean down_stream);
GList*
sed_cube_river_path(Sed_cube c, Sed_riv river);
GList*
sed_find_river_path(Sed_cube c,
    Eh_ind_2* hinge_pos,
    double angle);
Eh_ind_2*
sed_find_river_mouth(Sed_cube c,
    Eh_ind_2* hinge_pos,
    double angle);
Eh_ind_2
get_offset_from_angle(double angle, double aspect);

int
get_path_exit_side(Eh_pt_2 pos_in_box, double angle, double dx, double dy);


int
is_river_mouth(Eh_ind_2* shore_pos, Sed_hinge_pt* dir);
Eh_ind_2
get_offset_from_angle(double angle, double aspect);
Eh_pt_2
get_path_exit_pos(Eh_pt_2 pos_in_box, double angle, double dx, double dy);
Eh_pt_2
get_path_entrance_pos(Eh_pt_2 exit_pos, double dx, double dy);
Eh_ind_2
get_shift_from_exit_pos(Eh_pt_2 exit_pos, double dx, double dy);
GList*
sed_get_river_path(Sed_cube c, GList* river);
GList*
sed_cube_find_line_path(Sed_cube c,
    Eh_ind_2* hinge_pos,
    double angle);

double
sed_cube_river_angle(Sed_cube c, gpointer river_id);
double
sed_cube_river_bedload(Sed_cube s, gpointer river_id);
Eh_ind_2
sed_cube_river_mouth(Sed_cube c, gpointer river_id);
Eh_ind_2
sed_cube_river_hinge(Sed_cube s, gpointer river_id);
Sed_hydro
sed_cube_river_hydro(Sed_cube c, gpointer river_id);
Sed_cube
sed_cube_river_set_hydro(Sed_cube c, gpointer river_id, Sed_hydro h);

Eh_dbl_grid
sed_get_floor_3_default(int floor_type, int n_x, int n_y);
double**
sed_scan_sea_level_curve(const char* file, gint* len, GError** err);

typedef enum {
    SED_BATHY_FILE_TYPE_2D_ASCII,
    SED_BATHY_FILE_TYPE_2D_BINARY,
    SED_BATHY_FILE_TYPE_1D_ASCII
}
Sed_bathy_type;


Eh_dbl_grid
sed_bathy_grid_scan(const char* file,
    double dx,
    double dy,
    GError** error);
Eh_dbl_grid
sed_bathy_grid_scan_2d_binary(const char* file,
    double dx,
    double dy,
    GError** error);
Eh_dbl_grid
sed_bathy_grid_scan_2d_ascii(const char* file,
    double dx,
    double dy,
    GError** error);
Eh_dbl_grid
sed_bathy_grid_scan_1d_ascii(const char* file,
    double dx,
    double dy,
    GError** error);

Eh_sequence*
sed_get_floor_sequence_2(const char* file,
    double* y,
    gssize n_y,
    GError** err);
Eh_sequence*
sed_get_floor_sequence_3(const char* file,
    double dx,
    double dy,
    GError** error);

Sed_cube
sed_cube_foreach_river(Sed_cube c, GFunc func, gpointer user_data);

Sed_cube
sed_cube_find_all_river_mouths(Sed_cube c);

gssize
sed_cube_write(FILE* fp, const Sed_cube p);
Sed_cube
sed_cube_read(FILE* fp);

gssize*
sed_cube_find_column_below(Sed_cube c, double z);
gssize*
sed_cube_find_column_above(Sed_cube c, double z);

int
sed_get_floor_vec(char* filename, double* x, int len, double* y, GError** error);
GArray*
sed_get_floor(char* filename, GArray* x, GError** error);

gssize
sed_cube_id(Sed_cube p, gssize i, gssize j);
Eh_ind_2
sed_cube_sub(Sed_cube p, gssize id);
gboolean
sed_cube_is_in_domain(Sed_cube p, gssize i, gssize j);
gboolean
sed_cube_is_in_domain_id(Sed_cube p, gssize id);
gboolean
sed_cube_is_in_domain_pos(Sed_cube p, double x, double y);
gboolean
sed_cube_is_1d(Sed_cube p);

gboolean
is_shore_cell(Sed_cube s, gint x, gint y);
gboolean
is_shore_cell_id(Sed_cube s, gint id);
gboolean
is_coast_cell(Sed_cube s, gint i, gint j);
gboolean
is_coast_cell_id(Sed_cube s, gint id);
gboolean
is_land_cell(const Sed_cube s, gint i, gint j);
gboolean
is_land_cell_id(const Sed_cube s, gint id);
gboolean
is_ocean_cell(const Sed_cube s, gint i, gint j);
gboolean
is_ocean_cell_id(const Sed_cube s, gint id);
gboolean
is_boundary_cell_id(const Sed_cube s, gint id);

gssize
sed_cube_fprint(FILE* fp, Sed_cube c);

Sed_cell
sed_cube_to_cell(Sed_cube c, Sed_cell d);

gint
sed_cube_count_above(Sed_cube c, double h);
double
sed_cube_area_above(Sed_cube c, double h);

Sed_cube
sed_cube_deposit(Sed_cube c, Sed_cell* dz);
Sed_cube
sed_cube_erode(Sed_cube c, double* dz);
Sed_cube
sed_cube_deposit_cell(Sed_cube c,
    double const* dz,
    Sed_cell const add_cell);
Sed_cube
sed_cube_col_deposit_equal_amounts(Sed_cube c, gint id, double t);

double
sed_cube_get_angle(const Sed_cube c, const int start[2], const int end[2]);
G_END_DECLS

#endif

