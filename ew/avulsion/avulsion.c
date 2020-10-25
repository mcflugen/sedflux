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

#include <math.h>
#include <glib.h>
#include "utils/utils.h"
#include "avulsion.h"

GQuark
avulsion_data_struct_quark(void)
{
    return g_quark_from_static_string("avulsion-data-struct-quark");
}

Avulsion_st*
avulsion_new(GRand* rand, double std_dev)
{
    Avulsion_st* data = eh_new(Avulsion_st, 1);

    data->rand    = rand;
    data->std_dev = std_dev;

    return data;
}

Avulsion_st*
avulsion_dup(Avulsion_st* s)
{
    Avulsion_st* d = NULL;

    if (s) {
        d = avulsion_new(g_rand_copy(s->rand), s->std_dev);
    }

    return d;
}

Avulsion_st*
avulsion_destroy(Avulsion_st* data)
{
    if (data) {
        g_rand_free(data->rand);
        data->std_dev = 0;
        data->rand    = NULL;
        eh_free(data);
    }

    return NULL;
}

double
get_std_dev_func(double angle, double std_dev)
{
    return std_dev * (1. - .25 * exp(-pow(angle - .0 * G_PI, 2.)));
}

double
avulsion(GRand* rand, double last_angle, double std_dev)
{
    double new_angle;
    //   double d_angle = eh_rand_normal( rand , 0. , std_dev );
    double d_angle = eh_rand_normal(rand, 0., get_std_dev_func(last_angle, std_dev));

    new_angle = last_angle + d_angle;

    if (new_angle > G_PI) {
        new_angle = 2.*G_PI - new_angle;
    } else if (new_angle < -G_PI) {
        new_angle = -2.*G_PI - new_angle;
    }

    new_angle = eh_reduce_angle(new_angle);

    return new_angle;
}

double
avulsion_scale_angle_down(double angle, double min_angle, double max_angle)
{
    return   angle * (max_angle - min_angle) / (2.*G_PI) + .5 * (min_angle + max_angle);
}

double
avulsion_scale_angle_up(double angle, double min_angle, double max_angle)
{
    return (angle - .5 * (min_angle + max_angle)) * (2.*G_PI) / (max_angle - min_angle);
}

double
avulsion_scale_std_dev_up(double std_dev, double min_angle, double max_angle)
{
    return std_dev * 2.*G_PI / (max_angle - min_angle);
}

Sed_riv
sed_river_set_avulsion_data(Sed_riv r, Avulsion_st* data)
{
    g_dataset_id_set_data_full(r, AVULSION_DATA, data, (GDestroyNotify)avulsion_destroy);
    return r;
}

Sed_riv
sed_river_impart_avulsion_data(Sed_riv r)
{
    Sed_riv left = sed_river_left(r);
    Sed_riv right = sed_river_right(r);
    Avulsion_st* parent_data = sed_river_avulsion_data(r);

    if (left) {
        if (sed_river_avulsion_data(left) == NULL) {
            Avulsion_st* left_data = avulsion_dup(parent_data);
            left_data->std_dev  *= .5;
            left_data->rand = g_rand_new_with_seed(g_rand_int(parent_data->rand));
            sed_river_set_avulsion_data(left, left_data);
        } else {
            sed_river_impart_avulsion_data(left);
        }
    }

    if (right) {
        if (sed_river_avulsion_data(right) == NULL) {
            Avulsion_st* right_data = avulsion_dup(parent_data);
            right_data->std_dev  *= .5;
            right_data->rand = g_rand_new_with_seed(g_rand_int(parent_data->rand));
            sed_river_set_avulsion_data(right, right_data);
        } else {
            sed_river_impart_avulsion_data(right);
        }
    }

    /*
       Avulsion_st* parent_data = sed_river_avulsion_data(r);
       Avulsion_st* left_data   = avulsion_dup( parent_data );
       Avulsion_st* right_data  = avulsion_dup( parent_data );

       left_data->std_dev  *=.5;
       right_data->std_dev *=.5;

    eh_watch_ptr (sed_river_left (r));
    eh_watch_ptr (left_data);
    eh_watch_ptr (sed_river_right (r));
    eh_watch_ptr (right_data);
       sed_river_set_avulsion_data( sed_river_left (r) , left_data  );
       sed_river_set_avulsion_data( sed_river_right(r) , right_data );
    */
    return r;
}

Sed_riv
sed_river_unset_avulsion_data(Sed_riv r)
{
    g_dataset_id_remove_data(r, AVULSION_DATA);
    return r;
}

Avulsion_st*
sed_river_avulsion_data(Sed_riv r)
{
    return (Avulsion_st*)g_dataset_id_get_data(r, AVULSION_DATA);
}

Sed_riv
sed_river_avulse(Sed_riv r)
{
    eh_require(r);

    if (r) {
        double       angle;
        double       last_angle = sed_river_angle(r);
        Avulsion_st* data       = sed_river_avulsion_data(r);

        //eh_watch_ptr (r);
        eh_require(data);

        if (data && data->std_dev > 0) {
            GRand* rand       = data->rand;
            double std_dev    = data->std_dev;
            double min_angle  = sed_river_min_angle(r);
            double max_angle  = sed_river_max_angle(r);

            std_dev    = avulsion_scale_std_dev_up(std_dev, min_angle, max_angle);
            last_angle = avulsion_scale_angle_up(last_angle, min_angle, max_angle);
            angle      = avulsion(rand, last_angle, std_dev);
            angle      = avulsion_scale_angle_down(angle, min_angle, max_angle);

            sed_river_set_angle(r, angle);
        }
    }

    return r;
}

/** Avulse all of the branches of a Sed_riv

Run the avulsion process for each branch of a Sed_riv.  This will avulse all of the
branches, not just the smallest branches.  However, it is only the smallest branches
that enter the ocean.  We keep track of the location of the parent branches in
case its child branches merge.  In such a case the location of the merged river
will be the that of the parent.

\param c   A Sed_cube containing the river
\param r   A Sed_riv to avulse

\return The input Sed_cube

*/
Sed_cube
sed_cube_avulse_river(Sed_cube c, Sed_riv r)
{

    if (c) {
        Sed_riv* branch = sed_river_branches(r);
        Sed_riv* this_branch;

        for (this_branch = branch ; *this_branch ; this_branch++) {
            sed_river_avulse(*this_branch);
            sed_cube_find_river_mouth(c, *this_branch);
        }

        eh_free(branch);
    }

    return c;
}

void
sed_cube_avulse_river_helper(Sed_riv r, Sed_cube c)
{
    sed_cube_avulse_river(c, r);
}

Sed_cube
sed_cube_avulse_all_rivers(Sed_cube c)
{
    return sed_cube_foreach_river(c, (GFunc)&sed_cube_avulse_river_helper, c);
}

