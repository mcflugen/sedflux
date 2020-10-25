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

// Name :
//
// muds -- simulate the transport of muds in the bbl
//
//
// Synopsis :
//
// define            SKIN_DEPTH_PERCENT               ( 1. )
//
// Sed_cube*      muddy                            ( Sed_cube *prof,
//                                                      double kmax,
//                                                      double skin_depth,
//                                                      double dt )
// double            get_orbital_velocity             ( double d,
//                                                      double H,
//                                                      double T,
//                                                      double L )
//
// Description :
//
//  first we introduce hemiplagic muds into the bottom boundary layer.  from
//  wright et al., 2001 (equation 4 & 9) we can calculate the maximum load
//  that the gravity driven current can hold:
//
//                   2
//             Ri   u
//               cr  max
//  load_max = ---------
//                s g
//
//  where:
//    Ri    - richardson number
//    u_max - velocity at top of bbl
//    s     - submerged weight of sediment (~1.6)
//    g     - gravity
//
//  for locations where the bbl is over-full, the muds are deposited there.
//  for locations where the bbl is not yet full, and the near-bed velocities
//  are greater than the critical velocity for erosion, sediment is eroded
//  until the bbl is full or there is no more sediment to erode.
//
//  we now calculate the velocity at which the current will move down-slope
//  (wright et al., 2001 equation 12):
//
//          Ri Umax alpha     2       2     2
//    u  = --------------- , U     = u   + u
//     g         Cd           max     g     w
//
//  where:
//    ug    - down-slope gravity driven velocity
//    uw    - down-slope wave orbital velocity at bed
//    Umax  - total down-slope velocity
//    alpha - sea-floor slope
//    Cd    - drag coefficient
//
//  we now move the sediment down slope until there is no more sediment
//  suspended in the bbl.  for every iteration we use a variable time step
//  that is determined by:
//
//    dt = dx / u_max
//
//  where:
//    dt    - time step
//    dx    - grid spacing
//    u_max - maximum ug over the entire profile
//
//  this will be the maximum time step so that sediment will only travel a
//  maximum of one grid cell.
//
//

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "muds.h"

//#define ERODE_DEPTH_IN_YEARS (7./365.)
#define ERODE_DEPTH_IN_YEARS (200.)

#define sign( a ) ( (a)>0?1:((a)==0?0:-1) )
#define EROSION_IS_ON ( TRUE )

double
get_orbital_velocity(double d, double H, double T, double L);

int
test_in_suspension(double* in_suspension, double thickness, int n_grains);

int
muddy(Sed_cube prof, Sed_cell* in_suspension_cell, double* wave, double duration)
{
    int i, n;
    int n_grains;
    int i_start;
    gssize river_mouth;
    double s = 1.65, g = 9.81, Ri = .25, Cd = .003, u_wave_critical = .15;
    double dt, dx, alpha, beta;
    double extra, flux, initial_sediment, sediment_in_suspension;
    double wave_height, wave_period, wave_length;
    double max_erode_depth;
    double time_elapsed = 0.;
    double deposit_fraction, flux_fraction;
    double* max_load;
    double* u_max, *u_grav, *u_wave, u_grav_max;
    double* k_grain;
    double** in_suspension, *in_suspension_thickness;
    double** temp;
    Sed_cell temp_cell;

    wave_height = wave[0];
    wave_period = wave[1];
    // for deep water waves.
    //   wave_length = sed_gravity()*wave_period*wave_period/2/M_PI;

    // set the wave length so that the orbital velocities match at depth = wave_length/20.
    wave_length = 5.*sed_gravity() * pow(wave_period * sinh(M_PI / 10.) / M_PI, 2.);

    n_grains = sed_sediment_env_n_types();
    dx = sed_cube_y_res(prof);

    max_load = eh_new(double, sed_cube_n_y(prof));
    u_max    = eh_new(double, sed_cube_n_y(prof));
    u_grav   = eh_new(double, sed_cube_n_y(prof));
    u_wave   = eh_new(double, sed_cube_n_y(prof));

    // a k_grain of 1 will move all of the sediment possible.
    k_grain = sed_sediment_property(NULL, &sed_type_diff_coef);

    temp_cell     = sed_cell_new(n_grains);

    in_suspension_thickness = eh_new0(double, sed_cube_n_y(prof));
    in_suspension           = eh_new(double*, sed_cube_n_y(prof));
    in_suspension[0]        = eh_new(double, sed_cube_n_y(prof) * n_grains);
    temp                    = eh_new(double*, sed_cube_n_y(prof));
    temp[0]                 = eh_new0(double, sed_cube_n_y(prof) * n_grains);

    for (i = 1 ; i < sed_cube_n_y(prof) ; i++) {
        in_suspension[i] = in_suspension[i - 1] + n_grains;
        temp[i]          = temp[i - 1] + n_grains;
    }

    river_mouth = sed_cube_river_mouth_1d(prof);

    for (i = 0 ; i < river_mouth ; i++) {
        in_suspension_thickness[i] = 0.;

        for (n = 0 ; n < n_grains ; n++) {
            in_suspension[i][n] = 0.;
        }
    }

    for (i = river_mouth ; i < sed_cube_n_y(prof) ; i++) {
        in_suspension_thickness[i] = sed_cell_size(in_suspension_cell[i]);

        for (n = 0 ; n < n_grains ; n++)
            in_suspension[i][n] = sed_cell_fraction(in_suspension_cell[i], n)
                * in_suspension_thickness[i];
    }

    initial_sediment = 0.;

    for (i = 0 ; i < sed_cube_n_y(prof) ; i++) {
        initial_sediment += in_suspension_thickness[i];
    }

    do {
        river_mouth = sed_cube_river_mouth_1d(prof);

        // calculate the wave orbital velocities at the sea floor.
        for (i = 0 ; i < river_mouth ; i++) {
            u_wave[i] = 0.;
        }

        for (i = river_mouth ; i < sed_cube_n_y(prof) ; i++)
            u_wave[i] = get_orbital_velocity(sed_cube_water_depth(prof, 0, i),
                    wave_height,
                    wave_period,
                    wave_length);

        // determine the maximum load that a gravity driven current can sustain.
        for (i = 0 ; i < sed_cube_n_y(prof) - 1 ; i++) {
            alpha     = sed_cube_slope(prof, 0, i);
            beta      = Ri * alpha / Cd;

            if (fabs(beta) > .9) {
                beta = (beta > 0) ? .9 : -.9;
            }

            u_max[i]  = u_wave[i] / sqrt(1 - beta * beta);
            //u_max[i] = u_wave[i];
            u_grav[i] = beta * u_max[i];
        }

        u_max[i]  = u_max[i - 1];
        u_grav[i] = u_grav[i - 1];

        for (i = 0 ; i < sed_cube_n_y(prof) ; i++) {
            max_load[i] = Ri * u_max[i] * u_max[i] / s / g;
        }

        i_start = sed_cube_river_mouth_1d(prof);

        // check if the maximum load is exceeded.  if so, deposit the extra.  if
        // more space is available, and there is enough wave energy to erode
        // sediment then erode enough sediment to reach the maximum load (or as
        // much sediment as is available).
        for (i = i_start ; i < sed_cube_n_y(prof) ; i++) {
            extra = in_suspension_thickness[i] - max_load[i];

            // deposit the extra sediment.
            if (extra > 0 && in_suspension_thickness[i] > 0) {
                deposit_fraction = extra / in_suspension_thickness[i];

                for (n = 0 ; n < n_grains ; n++) {
                    in_suspension[i][n] *= deposit_fraction;
                }

                sed_column_add_vec(sed_cube_col(prof, i), in_suspension[i]);
                in_suspension_thickness[i] = 0.;

                for (n = 0 ; n < n_grains ; n++) {
                    in_suspension[i][n] /= deposit_fraction;
                    in_suspension[i][n] *= (1. - deposit_fraction);
                    in_suspension_thickness[i] += in_suspension[i][n];
                }

                //in_suspension_thickness[i] = max_load[i];
            }
            // erode sediment.
            else if (u_wave[i] > u_wave_critical && extra < 0 && EROSION_IS_ON) {
                extra *= -1.;

                max_erode_depth = sed_column_depth_age(sed_cube_col(prof, i),
                        sed_cube_age(prof)
                        - ERODE_DEPTH_IN_YEARS);
                //            max_erode_depth = .25;

                if (extra > max_erode_depth) {
                    extra = max_erode_depth;
                }

                if (extra > 0) {

                    //if ( extra>.25 )
                    //   extra = .25;

                    sed_column_extract_top(sed_cube_col(prof, i), extra, temp_cell);

                    for (n = 0 ; n < n_grains ; n++)
                        in_suspension[i][n] +=   sed_cell_fraction(temp_cell, n)
                            * sed_cell_size(temp_cell);

                    in_suspension_thickness[i] += sed_cell_size(temp_cell);
                }
            }
        }

        // determine the maximum u_grav in the profile and the largest time step for
        // stability.
        u_grav_max = 0.;

        for (i = i_start ; i < sed_cube_n_y(prof) ; i++) {
            flux = in_suspension_thickness[i] * u_grav[i];

            for (n = 0 ; n < n_grains ; n++)
                if (fabs(u_grav[i]*k_grain[n]) > u_grav_max && fabs(flux) > 0.) {
                    u_grav_max = fabs(u_grav[i] * k_grain[n]);
                }
        }

        if (u_grav_max <= 0.) {
            break;
        }

        dt = dx / u_grav_max;

        // now move the sediment still in suspension.
        for (i = i_start ; i < sed_cube_n_y(prof) - 1 ; i++) {
            flux_fraction = u_grav[i] * dt / dx;

            if (flux_fraction > 0 && u_grav[i] > 0)
                for (n = 0 ; n < n_grains ; n++) {
                    temp[i + 1][n]        += in_suspension[i][n] * flux_fraction * k_grain[n];
                    in_suspension[i][n] -= in_suspension[i][n] * flux_fraction * k_grain[n];
                } else if (flux_fraction < 0 && u_grav[i] < 0) {
                flux_fraction *= -1.;

                for (n = 0 ; n < n_grains ; n++) {
                    temp[i - 1][n]        += in_suspension[i][n] * flux_fraction * k_grain[n];
                    in_suspension[i][n] -= in_suspension[i][n] * flux_fraction * k_grain[n];
                }
            }
        }

        // do the last cell.
        flux_fraction = u_grav[i] * dt / dx;

        if (u_grav[i] < 0) {
            flux_fraction *= -1.;

            for (n = 0 ; n < n_grains ; n++) {
                temp[i - 1][n]        += in_suspension[i][n] * flux_fraction * k_grain[n];
                in_suspension[i][n] -= in_suspension[i][n] * flux_fraction * k_grain[n];
            }
        } else if (u_grav[i] > 0) {
            for (n = 0 ; n < n_grains ; n++) {
                in_suspension[i][n] -= in_suspension[i][n] * flux_fraction * k_grain[n];
            }
        }

        // do the first cell.
        flux_fraction = u_grav[0] * dt / dx;

        if (u_grav[0] > 0) {
            for (n = 0 ; n < n_grains ; n++) {
                temp[1][n]          += in_suspension[0][n] * flux_fraction * k_grain[n];
                in_suspension[0][n] -= in_suspension[0][n] * flux_fraction * k_grain[n];
            }
        } else
            for (n = 0 ; n < n_grains ; n++) {
                in_suspension[0][n] -= in_suspension[0][n] * flux_fraction * k_grain[n];
            }

        for (i = 0 ; i < sed_cube_n_y(prof) ; i++) {
            in_suspension_thickness[i] = 0.;

            for (n = 0 ; n < n_grains ; n++) {
                in_suspension[i][n]        += temp[i][n];
                in_suspension_thickness[i] += temp[i][n];
                temp[i][n]                  = 0.;
            }
        }

        sediment_in_suspension = 0.;

        for (i = 0 ; i < sed_cube_n_y(prof) - 1 ; i++) {
            sediment_in_suspension += in_suspension_thickness[i];
        }

        time_elapsed += dt;
    } while (sediment_in_suspension > .01 * initial_sediment && time_elapsed < duration);

    // deposit any sediment that might still be in suspension.
    for (i = 0 ; i < sed_cube_n_y(prof) ; i++) {
        sed_column_add_vec(sed_cube_col(prof, i), in_suspension[i]);
    }

    sed_cell_destroy(temp_cell);

    eh_free(in_suspension_thickness);
    eh_free(in_suspension[0]);
    eh_free(in_suspension);
    eh_free(temp[0]);
    eh_free(temp);

    eh_free(k_grain);
    eh_free(max_load);
    eh_free(u_max);
    eh_free(u_grav);
    eh_free(u_wave);

    return 0;
}

/** Calculate wave orbital velocity

   \f[
      u = \cases{ 0                                                      ,& $d<.5$\cr
                  {H\over 2}\sqrt{g\over d}                              ,& $.5<d<{\lambda\over 2}$\cr
                  {\pi H\over T \sinh\left({2\pi {d\over\lambda}}\right)},& $d>{\lambda\over 2}$\cr}
   \f]
\param d Water depth
\param H Wave height
\param T Wave period
\param L Wave length

\return The orbital velocity
*/
double
get_orbital_velocity(double d, double H, double T, double L)
{
    double g = 9.81;
    double u;

    if (H > d / 2.) {
        H = d / 2.;
    }

    if (d <= .5) {
        u = 0;
    } else if (d < L / 20.) {
        u = H / 2.*sqrt(g / d);
    }
    //   else if ( d<L/2 )
    else {
        u = M_PI * H / (T * sinh(2.*M_PI * d / L));
    }

    /*
       else
          u = 0.;
    */
    return u;
}

int
test_in_suspension(double* in_suspension, double thickness, int n_grains)
{
    int n;
    int error = 0;
    double total = 0;

    if (thickness > 1e-3) {
        for (n = 0 ; n < n_grains ; n++) {
            total += in_suspension[n];
        }

        if (fabs(total - thickness) / total > 1e-3) {
            error++;
        }

        if (error) {
            eh_watch_dbl(total);
            eh_watch_dbl(thickness);
        }
    }

    return error;
}
