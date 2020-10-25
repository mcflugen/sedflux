#if !defined (SUBSIDENCE_API_H)
#define SUBSIDENCE_API_H

#include <glib.h>
#include "subside.h"

G_BEGIN_DECLS

typedef struct {
    Eh_dbl_grid z; //< Elevation (m)
    Eh_dbl_grid load; //< Overlying load
    double eet; //< Effective elastic thickness (m)
    double youngs; //< Young's modulus
    double relaxation; //< Relaxation time (y)
    double rho_w; //< Density of water (kg/m3)
    double rho_m; //< Density of mantle (kg/m3)

    double time; //< The current time (y)
}
Subside_state;

/** Create and initialize a new Subside_state

@param nx Number of x nodes in computation grid
@param ny Number of y nodes in computation grid
@param dx Width of cells in the x direction
@param dy Width of cells in the y direction

@return A newly allocated Subside_state

*/
Subside_state*
sub_init(Subside_state* state, int nx, int ny, double dx, double dy);

/** Advance a Subside_state forward in time.

@param s A Subside_state
@param t Time (years) to advance model to.
*/
void
sub_run(Subside_state* s, double t);

/** Destroy a Subside_state

@param s A Subside_state
*/
void
sub_destroy(Subside_state* s);

/** Get deflection data

@param s A Subside_state

@return Array of deflections (in m)
*/
const double*
sub_get_deflection(Subside_state* s);
const double*
sub_get_load(Subside_state*);
const double*
sub_get_x(Subside_state*);
const double*
sub_get_y(Subside_state*);

int
sub_get_nx(Subside_state*);
int
sub_get_ny(Subside_state*);

void
sub_set_load(Subside_state*, double*);
void
sub_set_load_at(Subside_state* state, double load, int i, int j);
void
sub_set_eet(Subside_state*, double);
void
sub_set_youngs(Subside_state*, double);
void
sub_set_relax_time(Subside_state*, double);

G_END_DECLS

#endif

