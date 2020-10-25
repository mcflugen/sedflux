#include <stdio.h>
#include <utils/utils.h>
#include "subside_api.h"

Subside_state*
sub_init(Subside_state* state, int nx, int ny, double dx, double dy)
{
    if (state == NULL) {
        state = g_new(Subside_state, 1);
    }

    { /* Set default constants */
        state->eet = 5000.;
        state->youngs = 7e10;
        state->relaxation = 5000.;
        state->rho_w = 1030.;
        state->rho_m = 3300.;

        state->time = 0;
    }

    { /* Set up the initial profile */
        Eh_dbl_grid z = eh_grid_new(double, nx, ny);

        eh_grid_set_x_lin(z, 0, dx);
        eh_grid_set_y_lin(z, 0, dy);

        eh_dbl_grid_set(z, 0.);

        state->z = z;
    }

    { /* Set up the initial load */
        Eh_dbl_grid v = eh_grid_new(double, nx, ny);

        eh_grid_set_x_lin(v, 0, dx);
        eh_grid_set_y_lin(v, 0, dy);

        eh_dbl_grid_set(v, 0.);

        state->load = v;
    }

    return state;
}

void
sub_run(Subside_state* s, double t)
{
    subside_grid_load(s->z, s->load, s->eet, s->youngs);
    s->time = t;
}

void
sub_destroy(Subside_state* state)
{
    if (state) {
        eh_grid_destroy(state->z, TRUE);
        g_free(state);
    }
}

const double*
sub_get_deflection(Subside_state* state)
{
    eh_return_val_if_fail(state, NULL);
    return eh_dbl_grid_data_start(state->z);
}

const double*
sub_get_load(Subside_state* state)
{
    eh_return_val_if_fail(state, NULL);
    return eh_dbl_grid_data_start(state->load);
}

const double*
sub_get_x(Subside_state* state)
{
    eh_return_val_if_fail(state, NULL);
    return eh_grid_x(state->z);
}

const double*
sub_get_y(Subside_state* state)
{
    eh_return_val_if_fail(state, NULL);
    return eh_grid_y(state->z);
}

int
sub_get_nx(Subside_state* state)
{
    eh_return_val_if_fail(state, 0);
    return eh_grid_n_x(state->z);
}

int
sub_get_ny(Subside_state* state)
{
    eh_return_val_if_fail(state, 0);
    return eh_grid_n_y(state->z);
}

void
sub_set_load(Subside_state* state, double* load)
{
    eh_return_if_fail(state);
    eh_grid_borrow_data(state->load, (void*)load);
}

void
sub_set_load_at(Subside_state* state, double load, int i, int j)
{
    eh_return_if_fail(state);

    eh_dbl_grid_set_val(state->load, i, j, load);
}

void
sub_set_eet(Subside_state* state, double new_val)
{
    eh_return_if_fail(state);
    state->eet = new_val;
}

void
sub_set_youngs(Subside_state* state, double new_val)
{
    eh_return_if_fail(state);
    state->youngs = new_val;
}

void
sub_set_relax_time(Subside_state* state, double new_val)
{
    eh_return_if_fail(state);
    state->relaxation = new_val;
}

