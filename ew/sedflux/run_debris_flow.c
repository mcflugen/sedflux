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

#define SED_DEBRIS_FLOW_PROC_NAME "debris flow"
#define EH_LOG_DOMAIN SED_DEBRIS_FLOW_PROC_NAME

#include <stdio.h>
#include <stdlib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
//#include "failure.h"
#include "bing.h"
//#include "debris_flow.h"
#include "my_processes.h"

#define MAX_TRIES (1)

#define BING_LOCAL_MODEL
#undef BING_OLD_LOCAL_MODEL

Sed_process_info
run_debris_flow(Sed_process proc, Sed_cube p)
{
    Debris_flow_t*   data = (Debris_flow_t*)sed_process_user_data(proc);
    Sed_process_info info = SED_EMPTY_INFO;
    Sed_cell c, flow_cell;
    int i;
    int i_start, i_end;
    int tries = 0, max_tries = MAX_TRIES;
    double flow_rho, flow_age, flow_load;
    double head_start;
    pos_t* bathy, *flow;
    double* deposit;
    Sed_cube fail;
    bing_t bing_const;

    /*
       prof = sed_create_empty_profile( sed_cube_n_y(p) , p->sed );
       for ( i=0 ; i<prof->size ; i++ )
          prof->col[i] = p->col[0][i];
       prof->age = p->age;
       prof->time_step = p->time_step;
       prof->storm_value = p->storm_value;
       prof->quake_value = p->quake_value;
       prof->tidal_range = p->tidal_range;
       prof->tidal_period = p->tidal_period;
       prof->wave[0] = p->wave[0];
       prof->wave[1] = p->wave[1];
       prof->wave[2] = p->wave[2];
       prof->basinWidth = p->dx;
       prof->colWidth   = p->dy;
       prof->cellHeight = p->cell_height;
       prof->sealevel   = p->sea_level;
       prof->constants  = p->constants;
    */

    fail = (Sed_cube)sed_process_use(proc, FAILURE_PROFILE_DATA);
    //fail = data->failure;

    deposit = eh_new(double, sed_cube_n_y(p));

    // Define the new seafloor.
    bathy = createPosVec(sed_cube_n_y(p));

    for (i = 0; i < sed_cube_n_y(p); i++) {
        //      bathy->x[i] = p->col[i]->x*sed_get_profile_spacing(p);

        bathy->x[i] = sed_cube_col_y(p, i);
        bathy->y[i] = sed_cube_water_depth(p, 0, i);
    }

    // Define the thicknesses and positions for each slice in the
    // failure.
    flow = createPosVec(sed_cube_n_y(fail) - 2);

    for (i = 0; i < flow->size; i++) {
        flow->x[i] = sed_cube_col_y(fail, i + 1);
        flow->y[i] = sed_cube_thickness(fail, 0, i + 1);
    }

    head_start = flow->x[flow->size - 1];

    // Set the initial thicknesses of the debris flow nodes be uniform.  This
    // helps with stability of the debris flow module.
    eh_dbl_array_set(flow->y,
        flow->size,
        eh_dbl_array_mean(flow->y, flow->size));

    // Find the average density and age of the sediment in the failure.
    flow_cell = sed_cell_new_env();
    c         = sed_cell_new_env();

    for (i = 0; i < sed_cube_n_y(fail); i++) {
        sed_column_top(sed_cube_col(fail, i), sed_cube_thickness(fail, 0, i), c);
        sed_cell_add(flow_cell, c);
    }

    flow_rho  = sed_cell_density(flow_cell);
    flow_age  = sed_cell_age(flow_cell);
    flow_load = sed_cell_load(flow_cell) / sed_cube_n_y(fail);

    // define the bingham flow parameters.
    bing_const.numericalViscosity = data->numerical_viscosity;
    bing_const.dt                 = data->dt;
    bing_const.maxTime            = data->max_time;
    bing_const.flowDensity        = flow_rho;

#ifdef BING_LOCAL_MODEL   // the (new) local model.

    bing_const.yieldStrength = sed_cell_bulk_yield_strength(flow_cell);
    bing_const.viscosity     = sed_cell_bulk_dynamic_viscosity(flow_cell);

#else                     // the global model.

    bing_const.yieldStrength = data->yield_strength;
    bing_const.viscosity     = data->viscosity;

#endif

#ifdef BING_OLD_LOCAL_MODEL
    // the (old) local model.
    bing_const.yieldStrength = sed_cell_shear_strength(flow_cell, flow_load / 2.)
        / 5.;  // the sediment sensitivity.
    bing_const.viscosity     = sed_cell_viscosity(flow_cell) / 1.;

#endif

    // now we simulate the debris flow.  if there is an error running the
    // debris flow, increase the numerical velocity and keep trying until
    // it works or we reach max_ties.
    while (!bing(bathy, flow, bing_const, deposit) && (++tries) < max_tries) {
        bing_const.numericalViscosity += .1;
    }

    eh_message("time           : %f", sed_cube_age_in_years(p));
    eh_message("mass           : %f", sed_cube_mass(fail));
    eh_message("viscosity      : %f", bing_const.viscosity);
    eh_message("yield strength : %f", bing_const.yieldStrength);

    if (tries < max_tries) {
        // Now we add the debris flow sediment.
        sed_cell_set_facies(flow_cell, S_FACIES_DEBRIS_FLOW);
        sed_cell_set_age(flow_cell, flow_age);
        sed_cell_set_age(flow_cell, sed_cube_age_in_years(p));
        sed_cell_set_pressure(flow_cell, 0.);

        for (i = 0 ; i < sed_cube_n_y(p) ; i++)
            if (deposit[i] > 0) {
                sed_cell_resize(flow_cell, deposit[i]);
                sed_column_add_cell(sed_cube_col(p, i), flow_cell);
            }

        // find the location of the start and end of the deposit.
        for (i_start = 0 ; i_start < sed_cube_n_y(p) && deposit[i_start] <= 0. ; i_start++);

        for (i_end = sed_cube_n_y(p) - 1 ; i_end >= 0 && deposit[i_end] <= 0. ; i_end--);

        if (i_start == sed_cube_n_y(p)) {
            i_start = sed_cube_n_y(p) - 1;
            eh_require_not_reached();
        }

        if (i_end == -1) {
            i_end = 0;
            eh_require_not_reached();
        }

        eh_message("runout length  : %f",
            i_end * sed_cube_y_res(p) - head_start);
        eh_message("head start     : %f", head_start);
        eh_message("drop           : %f",
            sed_cube_water_depth(p, 0, i_end)
            - sed_cube_water_depth(p, 0, i_start));

    } else {
        eh_message("runout length  : -999");
        eh_message("head start     : -999");
        eh_message("drop           : -999");
        sed_cube_add(p, fail);
    }

    /*
       for ( i=0 ; i<prof->size ; i++ )
          sed_destroy_cell( prof->in_suspension[i] );
       eh_free(prof->in_suspension);
       sed_destroy_cell( prof->erode );
       eh_free(prof->col);
    */

    sed_cell_destroy(c);
    sed_cell_destroy(flow_cell);

    destroyPosVec(bathy);
    destroyPosVec(flow);
    eh_free(deposit);

    //   return ( tries<max_tries );
    return info;
}

#define S_KEY_YIELD_STRENGTH "yield strength"
#define S_KEY_VISCOSITY      "kinematic viscosity"
#define S_KEY_NUM_VISCOSITY  "artificial viscosity"
#define S_KEY_DT             "time step"
#define S_KEY_MAX_TIME       "maximum run time"

gboolean
init_debris_flow(Sed_process p, Eh_symbol_table tab, GError** error)
{
    Debris_flow_t* data    = sed_process_new_user_data(p, Debris_flow_t);
    GError*        tmp_err = NULL;
    gchar**        err_s   = NULL;
    gboolean       is_ok   = TRUE;

    eh_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    data->yield_strength      = eh_symbol_table_dbl_value(tab, S_KEY_YIELD_STRENGTH);
    data->viscosity           = eh_symbol_table_dbl_value(tab, S_KEY_VISCOSITY);
    data->numerical_viscosity = eh_symbol_table_dbl_value(tab, S_KEY_NUM_VISCOSITY);
    data->dt                  = eh_symbol_table_dbl_value(tab, S_KEY_DT);
    data->max_time            = eh_symbol_table_dbl_value(tab, S_KEY_MAX_TIME);

    eh_check_to_s(data->yield_strength >= 0, "Yield strength positive", &err_s);
    eh_check_to_s(data->viscosity >= 0, "Viscosity positive", &err_s);
    eh_check_to_s(data->numerical_viscosity >= 0, "Numerical viscosity positive", &err_s);
    eh_check_to_s(data->dt > 0, "Time step positive", &err_s);
    eh_check_to_s(data->max_time > 0, "Maximum run time positive", &err_s);

    // there is no failure sediment yet.
    data->failure = NULL;

    if (!tmp_err && err_s) {
        eh_set_error_strv(&tmp_err, SEDFLUX_ERROR, SEDFLUX_ERROR_BAD_PARAM, err_s);
    }

    if (tmp_err) {
        g_propagate_error(error, tmp_err);
        is_ok = FALSE;
    }

    return is_ok;
}

gboolean
destroy_debris_flow(Sed_process p)
{
    if (p) {
        Debris_flow_t* data = (Debris_flow_t*)sed_process_user_data(p);

        if (data) {
            eh_free(data);
        }
    }

    return TRUE;
}

gboolean
dump_debris_flow_data(gpointer ptr, FILE* fp)
{
    Debris_flow_t* data = (Debris_flow_t*)ptr;

    fwrite(data, sizeof(Debris_flow_t), 1, fp);
    sed_cube_write(fp, data->failure);

    return TRUE;
}

gboolean
load_debris_flow_data(gpointer ptr, FILE* fp)
{
    Debris_flow_t* data = (Debris_flow_t*)ptr;

    fread(data, sizeof(Debris_flow_t*), 1, fp);
    data->failure = sed_cube_read(fp);

    return TRUE;
}


