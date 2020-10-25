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

#ifndef _FAILURE_INCLUDED_
# define _FAILURE_INCLUDED_

#include <sed/sed_sedflux.h>
#include <glib.h>

G_BEGIN_DECLS

#ifndef MIN_FAILURE_LENGTH
    #define MIN_FAILURE_LENGTH (5)
#endif
#ifndef MAX_FAILURE_LENGTH
    #define MAX_FAILURE_LENGTH (100)
#endif
#ifndef MIN_FACTOR_OF_SAFETY
    #define MIN_FACTOR_OF_SAFETY (1.)
#endif

#define DECIDER_MAX_LENGTH        (2500.0)
#define DECIDER_THRESHOLD         (0.4)
#define DECIDER_TURBIDITY_CURRENT (0)
#define DECIDER_DEBRIS_FLOW       (1)
#define DECIDER_SLUMP             (2)

#define FAILURE_DEFAULT_CONSOLIDATION     (1e-6)
#define FAILURE_DEFAULT_COHESION          (1000.)
#define FAILURE_DEFAULT_FRICTION_ANGLE    (30.)
#define FAILURE_DEFAULT_GRAVITY           (9.81)
#define FAILURE_DEFAULT_DENSITY_SEA_WATER (1028.)

#define FAIL_FOS_NOT_VALID  (-1.)
#define FAIL_IGNORE_SURFACE (-2.)
#define FAIL_MIN_DELTA_H    (1e-5)

typedef struct {
    double depth;        // Depth of the failure slicei (m).
    double c;            // Sediment cohesion (Pa).
    double W;            // Weight of sediment (N/m).
    double b;            // Width of a slice (m).
    double u;            // Excess pore pressure (Pa).
    double phi;          // Coulomb friction angle (rads).
    double alpha;        // Slope at bottom of failure (rads).
    double fs;           // Factor of safety of the slice (-).
    double a_vertical;   // vertical ground acceleration (m/s/s).
    double a_horizontal; // horizontal ground acceleration (m/s/s).
} fs_t;

typedef struct {
    double depth;
    double c;
    double u;
    double w;
    double b;
    double phi;
    double alpha;
    double fs;
    double a_vertical;
    double a_horizontal;
}
Fail_slice;

typedef struct {
    double* w;                     // weight of the overlying sediment.
    double* u;                     // excess porewater pressure at the cell.
    double* phi;                   // average friction angle of overlying sediment.
    double* height;                // height to each cell.
    double* c;                     // average sediment cohesion.
    double fs[MAX_FAILURE_LENGTH]; // factor of safety for each ellipse.
    int size;                      // number of cells in column.
    int len;                       // number of cells allocated (length of w, u, etc).
    double failure_line;           // elevation to the bottom of the column.
    gboolean need_update;
}
Fail_column;

typedef struct {
    double consolidation;
    double cohesion;
    double frictionAngle;
    double gravity;
    double density_sea_water;
}
Failure_t;

typedef struct {
    Fail_column** col;    ///< failure data for each column.
    Sed_cube p;           ///< the underlying Sed_cube.
    double fs_min_val;    ///< value of the minimum factor of safety.
    int fs_min_start;     ///< start of the failure surface with min fos.
    int fs_min_len;       ///< length of the failure surface with min fos.
    Failure_t fail_const; ///< some constants for the profile.
    int size;             ///< number of columns.
    int len;              ///< number of columns allocated (length of col).
    int count;
}
Fail_profile;

void
factor_of_safety(double x, gconstpointer, double* fn, double* dfn);
double
rtsafe_fos(void (*funcd)(double, gconstpointer, double*, double*),
    gconstpointer data,
    double x1,
    double x2,
    double xacc);
Sed_cube
get_failure_surface(const Sed_cube, int, int);
double*
get_ellipse(const Sed_cube, int, int, double*);
double*
get_circle(const Sed_cube, int, int, double*);
double
get_factor_of_safety(const Sed_cube p,
    const Failure_t* failure_const);
int
decider(const Sed_cube, double);

Fail_column*
fail_create_fail_column(int n, gboolean allocate);
Fail_column*
fail_reuse_fail_column(Fail_column* f);
Fail_column*
fail_resize_fail_column(Fail_column* f, int n);
Fail_column*
fail_init_fail_column(Sed_column c,
    double h,
    Failure_t fail_const);
Fail_column*
fail_reinit_fail_column(Fail_column* f,
    Sed_column c,
    double h,
    Failure_t fail_const);
void
fail_destroy_fail_column(Fail_column* f);
void
fail_dump_fail_column(Fail_column* f, FILE* fp);
gboolean
fail_load_fail_column(Fail_column* f, FILE* fp);

Fail_profile*
fail_reuse_fail_profile(Fail_profile* f);
Fail_profile*
fail_create_fail_profile(int n_cols);
void
fail_dump_fail_profile(Fail_profile* f, FILE* fp);
void
fail_load_fail_profile(FILE* fp);
Fail_profile*
fail_resize_fail_profile(Fail_profile* f, int n_cols);
Fail_profile*
fail_reinit_fail_profile(Fail_profile* f,
    Sed_cube p,
    Failure_t fail_const);
Fail_profile*
fail_init_fail_profile(Sed_cube p,
    Failure_t fail_const);
void
fail_examine_fail_profile(Fail_profile* p);
void
fail_reset_fail_profile(Fail_profile* p);
void
fail_update_fail_profile(Fail_profile* p);

void
fail_destroy_failure_profile(Fail_profile* f);
double*
fail_get_failure_line(Sed_cube p);
double
fail_get_fail_profile_fos(Fail_profile* f, int start, int len);
void
get_node_fos(gpointer data, gpointer user_data);
gboolean
fail_fos_is_valid(double fos);
gboolean
fail_surface_is_valid(double h);
void
fail_set_failure_surface_ignore(Fail_profile* f, int start, int len);
gboolean
fail_get_ignore_surface(Fail_profile* f, int start, int len);
Fail_slice**
fail_get_janbu_parameters(Fail_profile* f,
    int start,
    double* fail_height,
    int fail_len);
gboolean
fail_check_failure_plane_is_valid(const Sed_cube p,
    int start,
    int len,
    const double* failure_plane);

G_END_DECLS

#endif /* failure.h is included */

