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

#include <stdio.h>

#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include <muds.h>

Sed_cell**
construct_deposit_array_3(Sed_cube   p,
    double     fraction,
    Sed_cell** deposit,
    Sed_riv    r);
int
rain_3(Sed_cube p, Sed_cell** deposit);
double
get_tidal_time_step(double t0,
    double tidal_range,
    double tidal_period,
    double dz);
double
get_tidal_time(double dz,
    double tidal_range,
    double tidal_period,
    gboolean waning);
double
get_tidal_level(double t, double tidal_range, double tidal_period);

gint
rain_sediment_3(Sed_cube p, int algorithm, Sed_riv this_river)
{
    gint error = 0;

    eh_require(p);
    eh_require(this_river);

    if (p && this_river) {
        Eh_ind_2 mouth_pos;

        this_river = sed_cube_find_river_mouth(p, this_river);
        mouth_pos  = sed_river_mouth(this_river);

        if (sed_cube_water_depth(p, mouth_pos.i, mouth_pos.j) >= 0) {
            const double  v_res         = sed_cube_z_res(p);
            const double  sea_level     = sed_cube_sea_level(p);
            const double  tidal_range   = sed_cube_tidal_range(p);
            const double  time_step     = sed_cube_time_step_in_days(p);
            Sed_cell      erode_cell    = sed_cell_new_env();
            Sed_cell_grid in_suspension = sed_cube_in_suspension(p, this_river);
            Sed_cell_grid deposit_grid  = sed_cell_grid_new_env(sed_cube_n_x(p), sed_cube_n_y(p));
            Sed_cell**    deposit       = (Sed_cell**)eh_grid_data(deposit_grid);
            double        time_elapsed  = 0.;
            double        time_left     = time_step;
            const double  tidal_period  = time_step;
            double fraction;
            double depth;
            double tidal_dt;
            double dz;
            double sediment_remaining;

            eh_require(erode_cell);
            eh_require(in_suspension);
            eh_require(deposit_grid);
            eh_require(deposit);

            // Add sediment to profile.  Start depositing at the river mouth.
            while (time_left > 1e-3
                && sed_cube_is_in_domain(p, mouth_pos.i, mouth_pos.j)
                && sed_cube_water_depth(p, mouth_pos.i, mouth_pos.j) > 0) {
                // The water depth at the river mouth.
                depth = sed_cube_water_depth(p, mouth_pos.i, mouth_pos.j) + 1e-5;

                // The fraction of sediment in the water column that will
                // be deposited.
                sediment_remaining = sed_cell_size(sed_cell_grid_val(in_suspension, 0, 0));

                if (sediment_remaining <= depth) {
                    fraction = 1.0;
                } else {
                    fraction = depth / sediment_remaining;
                }

                // if (sed_cell_size(sed_cell_grid_val(in_suspension, 0, 0)) > 0)
                //    fraction = depth / sed_cell_size(sed_cell_grid_val(in_suspension, 0, 0));
                // else
                //    fraction = 1.0;

                // eh_clamp( fraction , 1e-5 , 1. );

                if (tidal_range > 0) {
                    tidal_dt = get_tidal_time_step(time_elapsed,
                            tidal_range,
                            tidal_period,
                            v_res);

                    if (tidal_dt < time_left * fraction) {
                        fraction = tidal_dt / time_left;
                    }
                }

                // construct an array of cells to pass to the deposit routine.
                construct_deposit_array_3(p, fraction, deposit, this_river);

                // call the appropriate deposit routine.
                error = rain_3(p, deposit);

                // clear the deposit array.
                sed_cell_grid_clear(deposit_grid);

                // this is the time required to deposit this sediment.
                time_left    = time_left * (1. - fraction);
                time_elapsed = time_step - time_left;

                // get the new sea level.
                dz = get_tidal_level(time_elapsed, tidal_range, tidal_period);

                // adjust sea level and river mouth.
                sed_cube_set_sea_level(p, sea_level + dz);
                this_river = sed_cube_find_river_mouth(p, this_river);

                eh_require(this_river);

                mouth_pos = sed_river_mouth(this_river);
            }

            sed_cube_set_sea_level(p, sea_level);

            sed_cell_destroy(erode_cell);
            sed_cell_grid_destroy(deposit_grid);
        }
    }

    return error;
}

Sed_cell**
construct_deposit_array_3(Sed_cube   p,
    double     fraction,
    Sed_cell** deposit,
    Sed_riv    this_river)
{
    int i, j;
    double deposit_amount;
    double erode_amount;
    double remain_amount;
    double water_depth;
    Sed_cell erode_cell;
    Sed_cell_grid in_suspension;
    Eh_ind_2 mouth_pos;

    in_suspension = sed_cube_in_suspension(p, this_river);
    erode_cell    = sed_cell_new_env();

    mouth_pos = sed_river_mouth(this_river);

    for (i = 0 ; i < sed_cube_n_x(p) ; i++) {
        for (j = 0 ; j < sed_cube_n_y(p) ; j++) {
            deposit_amount = sed_cell_size(
                    sed_cell_grid_val(in_suspension, i - mouth_pos.i, j - mouth_pos.j))
                * fraction;

            //---
            // Any sediment that is deposited above sea level is now added to the
            // river sediment for the next time step.
            //---
            water_depth = sed_cube_water_depth(p, i, j);

            if (deposit_amount > 0
                && deposit_amount > water_depth - 1e-5) {
                if (water_depth < 0) {
                    water_depth = 0;
                }

                erode_amount   = deposit_amount - water_depth;
                deposit_amount = water_depth + 1e-5;

                sed_cell_copy(erode_cell,
                    sed_cell_grid_val(in_suspension, i - mouth_pos.i, j - mouth_pos.j));
                sed_cell_resize(erode_cell, erode_amount);
                //            sed_add_cell_to_cell( p->erode , erode_cell , sed_size( p->sed ) );

            }

            remain_amount = sed_cell_size(
                    sed_cell_grid_val(in_suspension, i - mouth_pos.i, j - mouth_pos.j))
                - deposit_amount;

            sed_cell_copy(deposit[i][j],
                sed_cell_grid_val(in_suspension, i - mouth_pos.i, j - mouth_pos.j));
            sed_cell_resize(deposit[i][j], deposit_amount);
            sed_cell_resize(sed_cell_grid_val(in_suspension, i - mouth_pos.i, j - mouth_pos.j),
                remain_amount);
            sed_cell_set_age(deposit[i][j], sed_cube_age(p));

        }
    }

    sed_cell_destroy(erode_cell);

    return deposit;
}

int
rain_3(Sed_cube p, Sed_cell** deposit)
{
    int i;
    gssize len = sed_cube_size(p);

    for (i = 0 ; i < len ; i++) {
        sed_column_add_cell(sed_cube_col(p, i), deposit[0][i]);
    }

    return 0;
}

//---
// calculate the amount of time that the elevation of the tide will change
// by an increment dz (m).  the current time is given as t0.
//---
double
get_tidal_time_step(double t0,
    double tidal_range,
    double tidal_period,
    double dz)
{
    double z0, t1;
    gboolean is_waning = FALSE;

    eh_require(t0 <= tidal_period);
    eh_require(t0 >= 0);

    z0 = get_tidal_level(t0, tidal_range, tidal_period);

    if (t0 > tidal_period / 2.) {
        dz *= -1;
        is_waning = TRUE;
    }

    if (fabs(z0 + dz) > tidal_range) {
        dz = 0;
        is_waning = is_waning ? FALSE : TRUE;
    }

    t1 = get_tidal_time(z0 + dz, tidal_range, tidal_period, is_waning);

    if (t1 < t0) {
        t1 += tidal_period;
    }

    if (t1 <= t0) {
        eh_watch_dbl(tidal_period);
        eh_watch_dbl(tidal_range);
        eh_watch_dbl(t1);
        eh_watch_dbl(t0);
    }

    eh_require(t1 > t0);

    return t1 - t0;
}

//---
// calculate the time required for the tide to reach a certain elevation, z.
// given a tidal range (m) and tidal period (s).  in order for there to be a
// single solution, we specify if the tide is waxing or waning.
//---
double
get_tidal_time(double z,
    double tidal_range,
    double tidal_period,
    gboolean waning)
{
    double dt;

    eh_require(fabs(z) <= tidal_range);

    dt = asin(z / tidal_range) * tidal_period / 2. / M_PI + tidal_period / 4.;

    if (waning) {
        dt = tidal_period - dt;
    }

    return dt;
}

//---
// calculate tide elevation at some time, t given a tidal range (m) and
// tidal period (s).
//---
double
get_tidal_level(double t, double tidal_range, double tidal_period)
{
    eh_require(t <= tidal_period);
    eh_require(t >= 0);
    t -= tidal_period / 4.;
    return tidal_range * sin(t * 2.*M_PI / tidal_period);
}


