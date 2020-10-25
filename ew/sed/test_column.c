#include <stdlib.h>
#include <glib.h>

#include "utils/utils.h"
#include "sed_column.h"

#include "test_sed.h"

void
test_sed_column_new(void)
{
    {
        Sed_column c = sed_column_new(5);

        g_assert(c != NULL);
        g_assert(sed_column_len(c) == 0);
        g_assert(fabs(sed_column_thickness(c)) < 1e-12);
        g_assert(fabs(sed_column_base_height(c)) <= 0);
        g_assert(fabs(sed_column_z_res(c) - 1) <= 0);
        g_assert(fabs(sed_column_x_position(c)) <= 0);
        g_assert(fabs(sed_column_y_position(c)) <= 0);

        sed_column_destroy(c);
    }
}


void
test_sed_column_new_neg(void)
{
    Sed_column c = sed_column_new(-1);

    g_assert(c == NULL);
}


void
test_sed_column_new_zero(void)
{
    Sed_column c = sed_column_new(0);

    g_assert(c == NULL);

}


void
test_sed_column_destroy(void)
{
    Sed_column c = sed_column_new(5);

    c = sed_column_destroy(c);

    g_assert(c == NULL);
}


void
test_sed_column_copy(void)
{
    Sed_column c_1 = sed_column_new(5);
    Sed_column c_2 = sed_column_new(55);
    Sed_column c_3;

    sed_column_set_base_height(c_1, 16);
    sed_column_set_z_res(c_1, 42);

    c_3 = sed_column_copy(c_2, c_1);

    g_assert(c_1 != c_2);
    g_assert(c_2 == c_3);
    g_assert(sed_column_is_same_data(c_1, c_2));
    g_assert(sed_column_is_same(c_1, c_2));

    sed_column_destroy(c_1);
    sed_column_destroy(c_2);
}


void
test_sed_column_copy_null(void)
{
    Sed_column c_1 = sed_column_new(5);
    Sed_column c_2;

    sed_column_set_base_height(c_1, 16);
    sed_column_set_z_res(c_1, 42);

    c_2 = sed_column_copy(NULL, c_1);

    g_assert(c_1 != NULL);
    g_assert(c_1 != c_2);
    g_assert(sed_column_is_same_data(c_1, c_2));
    g_assert(sed_column_is_same(c_1, c_2));

    sed_column_destroy(c_1);
    sed_column_destroy(c_2);
}


void
test_sed_column_clear(void)
{
    Sed_column c_0;
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 27.2, S_SED_TYPE_MUD);

    sed_column_set_z_res(c, 5);
    sed_column_set_x_position(c, 5);
    sed_column_set_y_position(c, 5);
    sed_column_set_base_height(c, 5);

    sed_column_add_cell(c, cell);

    c_0 = sed_column_clear(c);

    g_assert(c_0 == c);
    g_assert(sed_column_is_empty(c));
    g_assert(sed_column_mass_is(c, 0));
    g_assert(sed_column_len(c) == 0);

    if (fabs(sed_column_z_res(c)      - 5) > 1e-12
        || fabs(sed_column_x_position(c) - 5) > 1e-12
        || fabs(sed_column_y_position(c) - 5) > 1e-12
        || fabs(sed_column_base_height(c) - 5) > 1e-12) {
        g_assert_not_reached();
    }

    sed_column_destroy(c);
    sed_cell_destroy(cell);
}


void
test_sed_column_rebin(void)
{
    Sed_column c  = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 150.5, S_SED_TYPE_SAND);
    Sed_column c_0;
    double mass_0, mass_1;

    sed_column_stack_cell(c, cell);

    mass_0 = sed_column_mass(c);
    c_0    = sed_column_rebin(c);
    mass_1 = sed_column_mass(c);

    g_assert(c_0 == c);
    g_assert(fabs(mass_0 - mass_1) < 1e-12);
    g_assert(sed_column_len(c) == 151);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_destroy_null(void)
{
    Sed_column c = sed_column_destroy(NULL);

    g_assert(c == NULL);
}


void
test_sed_column_set_height(void)
{
    Sed_column c = sed_column_new(5);
    Sed_column c_0;

    c_0 = sed_column_set_base_height(c, 33);

    g_assert(c == c_0);
    g_assert(fabs(sed_column_base_height(c) - 33) < 1e-12);

    sed_column_destroy(c);
}


void
test_sed_column_height(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell s = sed_cell_new_classed(NULL, 10., S_SED_TYPE_SAND);
    double top, base;

    sed_column_set_base_height(c, 142);
    sed_column_add_cell(c, s);

    top  = sed_column_top_height(c);
    base = sed_column_base_height(c);
    g_assert(fabs(top - 152) < 1e-12);
    g_assert(fabs(base - 142) < 1e-12);

    sed_cell_destroy(s);
    sed_column_destroy(c);
}


void
test_sed_column_height_empty(void)
{
    Sed_column c = sed_column_new(5);
    double top, base;

    sed_column_set_base_height(c, 15);

    top  = sed_column_top_height(c);
    base = sed_column_base_height(c);

    g_assert(fabs(base - 15.) < 1e-12);
    g_assert(fabs(base - top) < 1e-12);
}


void
test_sed_column_height_null(void)
{
    double top  = sed_column_top_height(NULL);
    double base = sed_column_base_height(NULL);

    g_assert(fabs(top) < 1e-12);
    g_assert(fabs(base) < 1e-12);
}


void
test_sed_column_set_x_position(void)
{
    Sed_column c = sed_column_new(5);
    Sed_column c_0;

    c_0 = sed_column_set_x_position(c, 3.14);

    g_assert(c == c_0);
    g_assert(fabs(sed_column_x_position(c) - 3.14) < 1e-12);

    sed_column_destroy(c);
}


void
test_sed_column_set_y_position(void)
{
    Sed_column c = sed_column_new(5);
    Sed_column c_0;

    c_0 = sed_column_set_y_position(c, 2.78);

    g_assert(c == c_0);
    g_assert(fabs(sed_column_y_position(c) - 2.78) < 1e-12);

    sed_column_destroy(c);
}


void
test_sed_column_set_z_res(void)
{
    Sed_column c = sed_column_new(5);
    Sed_column c_0;

    c_0 = sed_column_set_z_res(c, .707);

    g_assert(c == c_0);
    g_assert(fabs(sed_column_z_res(c) - .707) < 1e-12);

    sed_column_destroy(c);
}


void
test_sed_column_stack_cells_loc(void)
{
    Sed_cell*  c_arr   = NULL;
    Sed_column c       = sed_column_new(5);
    double     mass_in = 0;
    gint       i;

    for (i = 0; i < 10; i++) {
        eh_strv_append(
            (gchar***)&c_arr,
            (gchar*)sed_cell_new_classed(NULL, 1., S_SED_TYPE_SAND));
        mass_in += sed_cell_mass(c_arr[i]);
    }

    sed_column_stack_cells_loc(c, c_arr);

    g_assert(sed_column_len(c) == g_strv_length((gchar**)c_arr));
    g_assert(eh_compare_dbl(sed_column_mass(c), mass_in, 1e-12));
    g_assert(eh_compare_dbl(sed_column_thickness(c), 10., 1e-12));

    for (i = 0 ; i < 10 ; i++) {
        g_assert(sed_column_nth_cell(c, i) == c_arr[i]);
    }

    eh_free(c_arr);
    sed_column_destroy(c);
}


void
test_sed_column_add_cell(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell s = sed_cell_new_classed(NULL, 1., S_SED_TYPE_SAND);
    double mass_in = sed_cell_mass(s);
    double t;

    t = sed_column_add_cell(c, s);

    g_assert(fabs(t - 1.) < 1e-12);
    g_assert(sed_column_len(c) == 1);
    g_assert(fabs(sed_cell_mass(s)  - mass_in) < 1e-12);
    g_assert(fabs(sed_column_mass(c) - mass_in) < 1e-12);
    g_assert(fabs(sed_column_thickness(c) - 1.) < 1e-12);

    sed_cell_resize(s, 128);
    mass_in += sed_cell_mass(s);
    t = sed_column_add_cell(c, s);

    g_assert(fabs(t - 128.) < 1e-12);
    g_assert(sed_column_len(c) == 129);
    g_assert(fabs(sed_column_mass(c) - mass_in) < 1e-12);
    g_assert(fabs(sed_column_thickness(c) - 129) < 1e-12);

    sed_cell_destroy(s);
    sed_column_destroy(c);
}


void
test_sed_column_add_cell_empty(void)
{
    double t;
    Sed_column c   = sed_column_new(5);
    Sed_cell s     = sed_cell_new_classed(NULL, 2., S_SED_TYPE_SAND);
    double mass_in = sed_cell_mass(s);

    t = sed_column_add_cell(c, s);
    sed_cell_resize(s, 0);
    t = sed_column_add_cell(c, s);

    g_assert(fabs(t) < 1e-12);
    g_assert(sed_column_len(c) == 2);
    g_assert(fabs(sed_column_mass(c) - mass_in) < 1e-12);
    g_assert(sed_column_size_is(c, 2.0));

    sed_cell_destroy(s);
    sed_column_destroy(c);
}


void
test_sed_column_add_cell_small(void)
{
    double t;
    gssize i;
    Sed_column c   = sed_column_new(5);
    Sed_cell s     = sed_cell_new_classed(NULL, .25, S_SED_TYPE_SAND);
    double mass_in = sed_cell_mass(s);

    t = sed_column_add_cell(c, s);

    g_assert(fabs(t - .25) < 1e-12);
    g_assert(sed_column_len(c) == 1);
    g_assert(fabs(sed_cell_mass(s)  - mass_in) < 1e-12);
    g_assert(fabs(sed_column_mass(c) - mass_in) < 1e-12);
    g_assert(fabs(sed_column_thickness(c) - .25) < 1e-12);

    sed_cell_resize(s, .030);

    for (i = 0, t = 0 ; i < 1000 ; i++) {
        mass_in += sed_cell_mass(s);
        t       += sed_column_add_cell(c, s);
    }

    g_assert(fabs(t - 30.) < 1e-12);
    g_assert(sed_column_len(c) == 31);
    g_assert(eh_compare_dbl(sed_column_mass(c), mass_in, 1e-12));
    g_assert(fabs(sed_column_thickness(c) - 30.25) < 1e-12);

    sed_cell_destroy(s);
    sed_column_destroy(c);
}


void
test_sed_column_add_cell_large(void)
{
    double t;
    gssize i;
    Sed_column c   = sed_column_new(5);
    Sed_cell s     = sed_cell_new_classed(NULL, 25, S_SED_TYPE_SAND);
    double mass_in = sed_cell_mass(s);

    t = sed_column_add_cell(c, s);

    g_assert(fabs(t - 25) < 1e-12);
    g_assert(sed_column_len(c) == 25);
    g_assert(fabs(sed_cell_mass(s)  - mass_in) < 1e-12);
    g_assert(fabs(sed_column_mass(c) - mass_in) < 1e-12);
    g_assert(fabs(sed_column_thickness(c) - 25.) < 1e-12);

    sed_cell_resize(s, 30);

    for (i = 0, t = 0 ; i < 1000 ; i++) {
        mass_in += sed_cell_mass(s);
        t       += sed_column_add_cell(c, s);
    }

    g_assert(fabs(t - 30000.) < 1e-12);
    g_assert(sed_column_len(c) == 30025);
    g_assert(fabs(sed_column_mass(c) - mass_in) < 1e-12);
    g_assert(fabs(sed_column_thickness(c) - 30025) < 1e-12);

    sed_cell_destroy(s);
    sed_column_destroy(c);
}


void
test_sed_column_stack_cell(void)
{
    Sed_column c   = sed_column_new(5);
    Sed_cell s     = sed_cell_new_classed(NULL, .5, S_SED_TYPE_SAND);
    double mass_in = sed_cell_mass(s);
    double t;

    t = sed_column_stack_cell(c, s);

    g_assert(fabs(t - .5) < 1e-12);
    g_assert(sed_column_len(c) == 1);
    g_assert(fabs(sed_cell_mass(s)  - mass_in) < 1e-12);
    g_assert(fabs(sed_column_mass(c) - mass_in) < 1e-12);
    g_assert(fabs(sed_column_thickness(c) - .5) < 1e-12);

    sed_cell_resize(s, 128);
    mass_in += sed_cell_mass(s);
    t = sed_column_stack_cell(c, s);

    g_assert(fabs(t - 128.) < 1e-12);
    g_assert(sed_column_len(c) == 2);
    g_assert(eh_compare_dbl(sed_column_mass(c), mass_in, 1e-12));
    g_assert(fabs(sed_column_thickness(c) - 128.5) < 1e-12);

    sed_cell_destroy(s);
    sed_column_destroy(c);
}


void
test_sed_column_compact_cell(void)
{
    Sed_column c_0 ;
    Sed_column c = sed_column_new(5);
    Sed_cell s   = sed_cell_new_classed(NULL, 1.5, S_SED_TYPE_SAND);

    sed_column_add_cell(c, s);

    c_0 = sed_column_compact_cell(c, 0, .5);

    g_assert(c_0 == c);
    g_assert(sed_cell_is_size(s, 1.5));
    g_assert(sed_column_size_is(c, 1.));
    g_assert(sed_cell_is_size(sed_column_nth_cell(c, 0), .5));

    sed_column_destroy(c);
    sed_cell_destroy(s);
}


void
test_sed_column_resize_cell(void)
{
    Sed_column c_0 ;
    Sed_column c = sed_column_new(5);
    Sed_cell s   = sed_cell_new_classed(NULL, 1.5, S_SED_TYPE_SAND);

    sed_column_add_cell(c, s);

    c_0 = sed_column_resize_cell(c, 0, .5);

    g_assert(c_0 == c);
    g_assert(sed_cell_is_size(s, 1.5));
    g_assert(sed_column_size_is(c, 1.));
    g_assert(sed_cell_is_size(sed_column_nth_cell(c, 0), .5));

    sed_column_destroy(c);
    sed_cell_destroy(s);
}


void
test_sed_column_resize_cell_neg(void)
{
    Sed_column c_0 ;
    Sed_column c = sed_column_new(5);
    Sed_cell s   = sed_cell_new_classed(NULL, 1.5, S_SED_TYPE_SAND);

    sed_column_add_cell(c, s);

    c_0 = sed_column_resize_cell(c, 0, -.5);

    g_assert(c_0 == c);
    g_assert(sed_column_size_is(c, .5));

    sed_column_destroy(c);
    sed_cell_destroy(s);
}


void
test_sed_column_resize_cell_over(void)
{
    Sed_column c_0 ;
    Sed_column c = sed_column_new(5);
    Sed_cell s   = sed_cell_new_classed(NULL, 1.5, S_SED_TYPE_SAND);

    sed_column_add_cell(c, s);

    c_0 = sed_column_resize_cell(c, 2, .5);

    g_assert(c_0 == c);
    g_assert(sed_column_size_is(c, 1.5));

    sed_column_destroy(c);
    sed_cell_destroy(s);
}


void
test_sed_column_resize_cell_under(void)
{
    Sed_column c_0 ;
    Sed_column c = sed_column_new(5);
    Sed_cell s   = sed_cell_new_classed(NULL, 1.5, S_SED_TYPE_SAND);

    sed_column_add_cell(c, s);

    c_0 = sed_column_resize_cell(c, -1, .5);

    g_assert(c_0 == c);
    g_assert(sed_column_size_is(c, 1.5));

    sed_column_destroy(c);
    sed_cell_destroy(s);
}


void
test_sed_column_nth_cell(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell s   = sed_cell_new_classed(NULL, 1., S_SED_TYPE_SAND);
    gssize i;

    for (i = 0 ; i < 10 ; i++) {
        sed_cell_set_age(s, i);
        sed_column_add_cell(c, s);
    }

    for (i = 0 ; i < sed_column_len(c) ; i++) {
        sed_cell_set_age(s, i);
        g_assert(sed_cell_is_same(sed_column_nth_cell(c, i), s));
    }

    sed_cell_destroy(s);
    sed_column_destroy(c);
}


void
test_sed_column_nth_cell_empty(void)
{
    Sed_column c = sed_column_new(5);

    g_assert(sed_cell_is_clear(sed_column_nth_cell(c, 0)));
    g_assert(sed_column_nth_cell(c, 1) == NULL);

    sed_column_destroy(c);
}


void
test_sed_column_nth_cell_null(void)
{
    g_assert(sed_column_nth_cell(NULL, 0) == NULL);
}


void
test_sed_column_top_cell(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell s   = sed_cell_new_classed(NULL, 12.5, S_SED_TYPE_SAND);
    Sed_cell top;

    sed_column_add_cell(c, s);

    top = sed_column_top_cell(c);

    g_assert(sed_column_nth_cell(c, 12) == top);

    sed_column_clear(c);
    top = sed_column_top_cell(c);

    g_assert(top == NULL);

    sed_column_destroy(c);
}


void
test_sed_column_total_load(void)
{
    Sed_column c  = sed_column_new(15);
    Sed_cell cell = sed_cell_new_classed(NULL, 26., S_SED_TYPE_SILT);
    double cell_load = sed_cell_load(cell);
    double* load = eh_new(double, 26);
    double load_0 = 2006;
    gssize i;

    sed_column_add_cell(c, cell);

    sed_column_total_load(c, 0, sed_column_len(c), load_0, load);

    for (i = 25 ; i >= 0 ; i--) {
        g_assert(fabs(load[i] - cell_load * (25 - i) + load_0) > 1e-12);
    }

    eh_free(load);
    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_load_at(void)
{
    Sed_column c  = sed_column_new(15);
    Sed_cell cell = sed_cell_new_classed(NULL, 26., S_SED_TYPE_SILT);
    double cell_load = sed_cell_load(cell);
    double load;

    sed_column_add_cell(c, cell);

    load = sed_column_load_at(c, 0);
    g_assert(fabs(load - cell_load * 25.) > 1e-12);

    load = sed_column_load_at(c, sed_column_top_index(c));
    g_assert(fabs(load) > 1e-12);

    load = sed_column_load_at(c, sed_column_top_index(c) - 5);
    g_assert(fabs(load - cell_load * 5) > 1e-12);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_load(void)
{
    Sed_column c  = sed_column_new(15);
    Sed_cell cell = sed_cell_new_classed(NULL, 26., S_SED_TYPE_SILT);
    double cell_load = sed_cell_load(cell);
    double* load = eh_new(double, 26);
    double* load_0;
    gssize i;

    sed_column_add_cell(c, cell);

    load_0 = sed_column_load(c, 0, sed_column_len(c), load);

    g_assert(load_0 == load);

    for (i = 25; i >= 0; i--) {
        g_assert(fabs(load[i] - cell_load * (25 - i)) > 1e-12);
    }

    eh_free(load);
    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_load_neg_start(void)
{
    Sed_column c  = sed_column_new(15);
    Sed_cell cell = sed_cell_new_classed(NULL, 26., S_SED_TYPE_SILT);
    double cell_load = sed_cell_load(cell);
    double* load = eh_new(double, 26);
    gssize i;

    sed_column_add_cell(c, cell);

    sed_column_load(c, -16, sed_column_len(c), load);

    for (i = 25 ; i >= 0 ; i--) {
        g_assert(fabs(load[i] - cell_load * (25 - i)) > 1e-12);
    }

    eh_free(load);
    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_load_neg_bins(void)
{
    Sed_column c  = sed_column_new(15);
    Sed_cell cell = sed_cell_new_classed(NULL, 26., S_SED_TYPE_SILT);
    double cell_load = sed_cell_load(cell);
    double* load = eh_new(double, 26);
    gssize i;

    sed_column_add_cell(c, cell);

    sed_column_load(c, -16, -1, load);

    for (i = 25 ; i >= 0 ; i--) {
        g_assert(fabs(load[i] - cell_load * (25 - i)) > 1e-12);
    }

    eh_free(load);
    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_load_partial(void)
{
    Sed_column c  = sed_column_new(15);
    Sed_cell cell = sed_cell_new_classed(NULL, 26., S_SED_TYPE_SILT);
    double cell_load = sed_cell_load(cell);
    double* load = eh_new(double, 5);
    gssize i;

    sed_column_add_cell(c, cell);

    sed_column_load(c, 0, 5, load);

    for (i = 4 ; i >= 0 ; i--) {
        g_assert(fabs(load[i] - cell_load * (25 - i)) > 1e-12);
    }

    eh_free(load);
    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_load_null(void)
{
    Sed_column c  = sed_column_new(15);
    Sed_cell cell = sed_cell_new_classed(NULL, 26., S_SED_TYPE_SILT);
    double cell_load = sed_cell_load(cell);
    double* load;
    gssize i;

    sed_column_add_cell(c, cell);

    load = sed_column_load(c, 0, sed_column_len(c), NULL);

    g_assert(load != NULL);

    for (i = 25 ; i >= 0 ; i--) {
        g_assert(fabs(load[i] - cell_load * (25 - i)) > 1e-12);
    }

    eh_free(load);
    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_top_index(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 3.14, S_SED_TYPE_SAND | S_SED_TYPE_SILT);
    gssize top_ind;

    sed_column_add_cell(c, cell);

    top_ind = sed_column_top_index(c);

    g_assert(top_ind == 3);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_top_index_null(void)
{
    gssize top_ind;

    top_ind = sed_column_top_index(NULL);

    g_assert(top_ind == -1);
}


void
test_sed_column_top_index_empty(void)
{
    Sed_column c = sed_column_new(5);
    gssize top_ind;

    top_ind = sed_column_top_index(c);

    g_assert(top_ind == -1);

    sed_column_destroy(c);
}


void
test_sed_column_is_above(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 33, S_SED_TYPE_SAND);

    sed_column_set_base_height(c, 58);
    sed_column_add_cell(c, cell);

    g_assert(sed_column_is_above(c, 90));
    g_assert(!sed_column_is_above(c, 92));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_is_below(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 33, S_SED_TYPE_SAND);

    sed_column_set_base_height(c, 58);
    sed_column_add_cell(c, cell);

    g_assert(!sed_column_is_below(c, 90));
    g_assert(sed_column_is_below(c, 92));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_is_valid_index(void)
{
    Sed_column c = sed_column_new(5);

    g_assert(sed_column_is_valid_index(c, 0));
    g_assert(!sed_column_is_valid_index(c, S_ADDBINS));
    g_assert(!sed_column_is_valid_index(c, -1));

    g_assert(!sed_column_is_valid_index(NULL, 0));

    sed_column_clear(c);

    g_assert(sed_column_is_valid_index(c, 0));

    sed_column_destroy(c);
}


void
test_sed_column_is_get_index(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 6, S_SED_TYPE_SAND);
    gssize len;

    sed_column_add_cell(c, cell);
    len = sed_column_len(c);

    g_assert(sed_column_is_get_index(c, 0));
    g_assert(!sed_column_is_get_index(c, len));
    g_assert(sed_column_is_get_index(c, len - 1));
    g_assert(!sed_column_is_get_index(c, -1));

    g_assert(!sed_column_is_get_index(NULL, 0));

    sed_column_clear(c);

    g_assert(!sed_column_is_get_index(c, 0));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_is_set_index(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 6, S_SED_TYPE_SAND);
    gssize len;

    sed_column_add_cell(c, cell);
    len = sed_column_len(c);

    g_assert(sed_column_is_set_index(c, 0));
    g_assert(sed_column_is_set_index(c, len));
    g_assert(sed_column_is_set_index(c, len - 1));
    g_assert(!sed_column_is_set_index(c, -1));

    g_assert(!sed_column_is_get_index(NULL, 0));

    sed_column_clear(c);

    g_assert(sed_column_is_set_index(c, 0));
    g_assert(!sed_column_is_set_index(c, 1));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_top_nbins(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 50., S_SED_TYPE_SAND);
    gssize n;

    sed_column_set_base_height(c, 100);
    sed_column_add_cell(c, cell);

    n = sed_column_top_nbins(c, 150);
    g_assert(n == 1);

    n = sed_column_top_nbins(c, 148.5);
    g_assert(n == 2);

    n = sed_column_top_nbins(c, 100);
    g_assert(n == sed_column_len(c));

    n = sed_column_top_nbins(c, 99);
    g_assert(n == sed_column_len(c));

    n = sed_column_top_nbins(c, 151);
    g_assert(n == 1);

    sed_cell_destroy(cell);
    sed_column_destroy(c);

}


void
test_sed_column_top(void)
{
    Sed_column c = sed_column_new_filled(20, S_SED_TYPE_SAND);
    Sed_cell cell = sed_cell_new_classed(NULL, 13, S_SED_TYPE_CLAY);
    Sed_cell cell_0;

    cell_0 = sed_column_top(c, 1.5, cell);

    g_assert(cell_0 == cell);
    g_assert(sed_cell_is_size(cell, 1.5));
    g_assert(sed_cell_is_size_class(cell, S_SED_TYPE_SAND));
    g_assert(sed_column_size_is(c, 20));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_top_above(void)
{
    Sed_column c = sed_column_new_filled(20, S_SED_TYPE_SAND);
    Sed_cell cell = sed_cell_new_classed(NULL, 13, S_SED_TYPE_CLAY);
    Sed_cell cell_0;

    cell_0 = sed_column_top(c, -1.5, cell);

    g_assert(cell_0 == cell);
    g_assert(sed_cell_is_clear(cell));
    g_assert(sed_column_size_is(c, 20));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_top_below(void)
{
    Sed_column c = sed_column_new_filled(20, S_SED_TYPE_SAND);
    Sed_cell cell = sed_cell_new_classed(NULL, 13, S_SED_TYPE_CLAY);
    Sed_cell cell_0;

    cell_0 = sed_column_top(c, 21.5, cell);

    g_assert(cell_0 == cell);
    g_assert(sed_cell_is_size(cell, 20));
    g_assert(sed_column_size_is(c, 20));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_top_null(void)
{
    Sed_column c = sed_column_new_filled(20, S_SED_TYPE_SAND);
    Sed_cell cell;

    cell = sed_column_top(c, 1.5, NULL);

    g_assert(sed_cell_is_valid(cell));
    g_assert(sed_cell_is_size(cell, 1.5));
    g_assert(sed_cell_is_size_class(cell, S_SED_TYPE_SAND));
    g_assert(sed_column_size_is(c, 20));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_top_empty(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 13, S_SED_TYPE_CLAY);
    Sed_cell cell_0;

    cell_0 = sed_column_top(c, 1.5, cell);

    g_assert(cell_0 == cell);
    g_assert(sed_cell_is_clear(cell));
    g_assert(sed_column_is_empty(c));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_top_rho(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 13, S_SED_TYPE_CLAY | S_SED_TYPE_SAND);
    double rho, rho_0;

    sed_column_add_cell(c, cell);

    rho_0 = sed_cell_density(cell);
    rho = sed_column_top_rho(c, 1.5);

    g_assert(eh_compare_dbl(rho, rho_0, 1e-12));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_top_age(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 1, S_SED_TYPE_CLAY | S_SED_TYPE_SAND);
    double age;
    gssize i;

    for (i = 1 ; i <= 10 ; i++) {
        sed_cell_set_age(cell, i);
        sed_column_add_cell(c, cell);
    }

    age = sed_column_top_age(c, 1.5);

    g_assert(eh_compare_dbl(age, 29. / 3., 1e-12));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_top_property(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 10, S_SED_TYPE_CLAY | S_SED_TYPE_SAND);
    Sed_property p = sed_property_new("grain");
    double gz, gz_0;

    sed_column_add_cell(c, cell);

    gz_0 = sed_cell_grain_size_in_phi(cell);
    gz   = sed_column_top_property(p, c, 1.5);

    g_assert(eh_compare_dbl(gz, gz_0, 1e-12));

    sed_property_destroy(p);
    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_top_property_with_load(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 10, S_SED_TYPE_CLAY | S_SED_TYPE_SAND);
    Sed_property p = sed_property_new("cohesion");
    double cohesion, cohesion_0;

    sed_column_add_cell_real(c, cell, FALSE);

    cohesion_0 = sed_cell_cohesion(cell, sed_cell_load(cell) * .1);
    cohesion   = sed_column_top_property(p, c, 1.);

    g_assert(eh_compare_dbl(cohesion, cohesion_0, 1e-12));

    cohesion_0 = sed_cell_cohesion(cell, sed_cell_load(cell) * .65);
    cohesion   = sed_column_top_property(p, c, 6.5);

    g_assert(eh_compare_dbl(cohesion, cohesion_0, 1e-12));

    sed_property_destroy(p);
    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_index_depth(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 21., S_SED_TYPE_SAND);
    gssize ind;

    sed_column_add_cell(c, cell);

    ind = sed_column_index_depth(c, 1.5);
    g_assert(ind == 19);

    ind = sed_column_index_depth(c, 2);
    g_assert(ind == 18);

    ind = sed_column_index_depth(c, 0);
    g_assert(ind == 20);

    ind = sed_column_index_depth(c, 21);
    g_assert(ind == -1);

    ind = sed_column_index_depth(c, 21. / 2.);
    g_assert(ind == 10);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_depth_age(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 1., S_SED_TYPE_SAND);
    double d;
    gssize i;

    for (i = 1 ; i <= 3 ; i++) {
        sed_cell_set_age(cell, i / 10.);
        sed_column_add_cell(c, cell);
    }

    d = sed_column_depth_age(c, .1);
    g_assert(eh_compare_dbl(d, 2, 1e-12));

    d = sed_column_depth_age(c, .3);
    g_assert(eh_compare_dbl(d, 0, 1e-12));

    d = sed_column_depth_age(c, .25);
    g_assert(eh_compare_dbl(d, 1., 1e-12));

    d = sed_column_depth_age(c, 0);
    g_assert(eh_compare_dbl(d, 3., 1e-12));

    d = sed_column_depth_age(c, .4);
    g_assert(eh_compare_dbl(d, 0, 1e-12));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_thickness_index(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 21., S_SED_TYPE_SAND);
    double t;

    sed_column_add_cell(c, cell);

    t = sed_column_thickness_index(c, 0);
    g_assert(fabs(t - 1.) < 1e-12);

    t = sed_column_thickness_index(c, 20);
    g_assert(fabs(t - 21.) < 1e-12);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_thickness_index_neg(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 21., S_SED_TYPE_SAND);
    double t;

    sed_column_add_cell(c, cell);

    t = sed_column_thickness_index(c, -1);
    g_assert(fabs(t) < 1e-12);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_thickness_index_above(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 21., S_SED_TYPE_SAND);
    double t;

    sed_column_add_cell(c, cell);

    t = sed_column_thickness_index(c, 47);
    g_assert(fabs(t - 21) < 1e-12);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_thickness_index_null(void)
{
    Sed_column c = sed_column_new(5);
    double t = sed_column_thickness_index(NULL, 0);

    g_assert(fabs(t) < 1e-12);

    sed_column_destroy(c);
}


void
test_sed_column_thickness_index_empty(void)
{
    double t = sed_column_thickness_index(NULL, 47);
    g_assert(fabs(t) < 1e-12);
}


void
test_sed_column_index_thickness(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 21., S_SED_TYPE_SAND);
    gssize ind;

    sed_column_add_cell(c, cell);

    ind = sed_column_index_thickness(c, 3.5);
    g_assert(ind == 3);

    ind = sed_column_index_thickness(c, 3);
    g_assert(ind == 2);

    ind = sed_column_index_thickness(c, 21);
    g_assert(ind == 20);

    ind = sed_column_index_thickness(c, 0);
    g_assert(ind == -1);

    ind = sed_column_index_thickness(c, 21. / 2.);
    g_assert(ind == 10);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_index_at(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 21., S_SED_TYPE_SAND);
    gssize ind;

    sed_column_set_base_height(c, 142);
    sed_column_add_cell(c, cell);

    ind = sed_column_index_at(c, 145.5);
    g_assert(ind == 3);

    ind = sed_column_index_at(c, 145);
    g_assert(ind == 2);

    ind = sed_column_index_at(c, 163);
    g_assert(ind == 20);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_index_at_base(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 21., S_SED_TYPE_SAND);
    gssize ind;

    sed_column_set_base_height(c, 142);
    sed_column_add_cell(c, cell);

    ind = sed_column_index_at(c, 142);
    g_assert(ind == -1);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_index_at_above(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 21., S_SED_TYPE_SAND);
    gssize ind;

    sed_column_set_base_height(c, 142);
    sed_column_add_cell(c, cell);

    ind = sed_column_index_at(c, 175);
    g_assert(ind == 20);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_index_at_below(void)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 21., S_SED_TYPE_SAND);
    gssize ind;

    sed_column_set_base_height(c, 142);
    sed_column_add_cell(c, cell);

    ind = sed_column_index_at(c, 100);
    g_assert(ind == -1);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


#include <glib/gstdio.h>

void
test_sed_column_write(void)
{
    char* name_used;
    gssize n_bytes;

    {
        Sed_column c = sed_column_new(5);
        Sed_cell cell = sed_cell_new_classed(NULL, 23.1, S_SED_TYPE_SAND | S_SED_TYPE_MUD);
        FILE* fp_tmp = eh_open_temp_file("sed_column_test.binXXXXXX", &name_used);

        sed_column_add_cell(c, cell);

        sed_column_set_z_res(c, 2.718);
        sed_column_set_x_position(c, 3.14);
        sed_column_set_y_position(c, 9.81);
        sed_column_set_base_height(c, 1.414);
        sed_column_set_age(c, 33.);

        n_bytes = sed_column_write(fp_tmp, c);

        sed_cell_destroy(cell);
        sed_column_destroy(c);

        fclose(fp_tmp);
    }

    {
        FILE* fp;
        FILE* fp_tmp = fopen(name_used, "rb");
        char* data_0 = eh_new(char, n_bytes);
        char* data_1 = eh_new(char, n_bytes);

        fp = fopen(SED_COLUMN_TEST_FILE, "rb");

        fread(data_0, sizeof(char), n_bytes, fp);
        fread(data_1, sizeof(char), n_bytes, fp_tmp);

        g_assert(strncmp(data_0, data_1, n_bytes) == 0);

        eh_free(data_0);
        eh_free(data_1);

        fclose(fp);
        fclose(fp_tmp);
    }

    g_remove(name_used);
}


void
test_sed_column_read(void)
{
    FILE* fp = eh_fopen(SED_COLUMN_TEST_FILE, "rb");
    Sed_column c;

    c = sed_column_read(fp);

    g_assert(c != NULL);
    g_assert(fabs(sed_column_z_res(c) - 2.718)       < 1e-12);
    g_assert(fabs(sed_column_x_position(c) - 3.14)   < 1e-12);
    g_assert(fabs(sed_column_y_position(c) - 9.81)   < 1e-12);
    g_assert(fabs(sed_column_base_height(c) - 1.414) < 1e-12);
    g_assert(fabs(sed_column_mass(c) - 40425)        < 1e0);
    g_assert(sed_column_len(c) == 24);

    sed_column_destroy(c);

    fclose(fp);
}


void
test_sed_column_chop(void)
{
    Sed_column c_0;
    Sed_column c  = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 20, S_SED_TYPE_SAND);

    sed_column_set_base_height(c, 123);

    sed_column_add_cell(c, cell);

    c_0 = sed_column_chop(c, 133);

    g_assert(c_0 == c);
    g_assert(fabs(sed_column_base_height(c) - 123) < 1e-23);
    g_assert(fabs(sed_column_top_height(c) - 133) < 1e-23);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_chomp(void)
{
    Sed_column c_0;
    Sed_column c  = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 20, S_SED_TYPE_SAND);

    sed_column_set_base_height(c, 123);

    sed_column_add_cell(c, cell);

    c_0 = sed_column_chomp(c, 133);

    g_assert(c_0 == c);
    g_assert(fabs(sed_column_base_height(c) - 133) < 1e-23);
    g_assert(fabs(sed_column_top_height(c) - 143) < 1e-23);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_strip(void)
{
    Sed_column c_0;
    Sed_column c  = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 20, S_SED_TYPE_SAND);

    sed_column_set_base_height(c, 123);

    sed_column_add_cell(c, cell);

    c_0 = sed_column_strip(c, 133, 135);

    g_assert(c_0 == c);
    g_assert(fabs(sed_column_base_height(c) - 133) < 1e-23);
    g_assert(fabs(sed_column_top_height(c) - 135) < 1e-23);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_chop_above(void)
{
    Sed_column c_0;
    Sed_column c  = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 20, S_SED_TYPE_SAND);

    sed_column_set_base_height(c, 123);

    sed_column_add_cell(c, cell);

    c_0 = sed_column_chop(c, 145);

    g_assert(c_0 == c);
    g_assert(fabs(sed_column_base_height(c) - 123) < 1e-23);
    g_assert(fabs(sed_column_top_height(c) - 143) < 1e-23);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_extract_above_0(void)
{
    Sed_cell* c_arr = NULL;
    Sed_column c  = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 20, S_SED_TYPE_SAND);

    sed_column_set_base_height(c, 123);

    sed_column_add_cell(c, cell);

    c_arr = sed_column_extract_cells_above(c, 140);

    g_assert(c_arr != NULL);
    g_assert(g_strv_length((gchar**)c_arr) == 3);
    g_assert(fabs(sed_column_base_height(c) - 123) < 1e-23);
    g_assert(fabs(sed_column_top_height(c) - 140) < 1e-23);

    sed_cell_array_free(c_arr);
    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_extract_above_1(void)
{
    Sed_cell* c_arr = NULL;
    Sed_column c  = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 20, S_SED_TYPE_SAND);

    sed_column_set_base_height(c, 123);

    sed_column_add_cell(c, cell);

    c_arr = sed_column_extract_cells_above(c, 125.1);

    g_assert(c_arr != NULL);
    g_assert(g_strv_length((gchar**)c_arr) == 18);
    g_assert(fabs(sed_column_base_height(c) - 123) < 1e-23);
    g_assert(fabs(sed_column_top_height(c) - 125.1) < 1e-23);

    sed_cell_array_free(c_arr);
    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_extract_above_2(void)
{
    Sed_cell* c_arr = NULL;
    Sed_column c  = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 20, S_SED_TYPE_SAND);

    sed_column_set_base_height(c, 123);

    sed_column_add_cell(c, cell);

    c_arr = sed_column_extract_cells_above(c, 120.1);

    g_assert(c_arr != NULL);
    g_assert(g_strv_length((gchar**)c_arr) == 20);
    g_assert(sed_column_is_empty(c));
    g_assert(fabs(sed_column_base_height(c) - 123) < 1e-23);
    g_assert(fabs(sed_column_top_height(c) - 123) < 1e-23);

    sed_cell_array_free(c_arr);
    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_chop_below(void)
{
    Sed_column c_0;
    Sed_column c  = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 20, S_SED_TYPE_SAND);

    sed_column_set_base_height(c, 123);

    sed_column_add_cell(c, cell);

    c_0 = sed_column_chop(c, 120);

    g_assert(c_0 == c);
    g_assert(fabs(sed_column_base_height(c) - 120) < 1e-23);
    g_assert(fabs(sed_column_top_height(c) - 120) < 1e-23);
    g_assert(sed_column_is_empty(c));

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_chop_null(void)
{
    g_assert(sed_column_chop(NULL, 120) == NULL);
}


void
test_sed_column_chomp_above(void)
{
    Sed_column c_0;
    Sed_column c  = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 20, S_SED_TYPE_SAND);

    sed_column_set_base_height(c, 123);

    sed_column_add_cell(c, cell);

    c_0 = sed_column_chomp(c, 153);

    g_assert(c_0 == c);
    g_assert(fabs(sed_column_base_height(c) - 153) < 1e-23);
    g_assert(fabs(sed_column_top_height(c) - 153) < 1e-23);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_chomp_below(void)
{
    Sed_column c_0;
    Sed_column c  = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 20, S_SED_TYPE_SAND);

    sed_column_set_base_height(c, 123);

    sed_column_add_cell(c, cell);

    c_0 = sed_column_chomp(c, 120);

    g_assert(c_0 == c);
    g_assert(fabs(sed_column_base_height(c) - 123) < 1e-23);
    g_assert(fabs(sed_column_top_height(c) - 143) < 1e-23);

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}


void
test_sed_column_chomp_null(void)
{
    g_assert(sed_column_chomp(NULL, 120) == NULL);
}


void
test_sed_cell_add_column(void)
{
    Sed_column s = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 10, S_SED_TYPE_SAND);
    Sed_cell cell_0;
    double mass_in;

    sed_cell_set_age(cell, 33);
    sed_column_add_cell(s, cell);

    sed_cell_set_age(cell, 66);
    sed_column_add_cell(s, cell);

    mass_in = sed_column_mass(s);

    sed_cell_clear(cell);
    cell_0 = sed_cell_add_column(cell, s);

    g_assert(cell_0 == cell);
    g_assert(sed_cell_is_valid(cell));
    g_assert(sed_cell_is_mass(cell, mass_in));
    g_assert(sed_cell_is_age(cell, 49.5));
    g_assert(sed_column_mass_is(s, mass_in));

    sed_cell_add_column(cell, s);

    g_assert(sed_cell_is_mass(cell, 2 * mass_in));

    sed_cell_destroy(cell);
    sed_column_destroy(s);
}


void
test_sed_cell_add_column_dup(void)
{
    Sed_column s = sed_column_new(5);
    Sed_cell cell = sed_cell_new_classed(NULL, 33, S_SED_TYPE_SAND);
    Sed_cell cell_0;
    double mass_in;

    sed_cell_set_age(cell, 33);
    sed_column_add_cell(s, cell);

    mass_in = sed_column_mass(s);

    cell_0 = sed_cell_add_column(NULL, s);

    g_assert(sed_cell_is_valid(cell_0));
    g_assert(sed_cell_is_mass(cell_0, mass_in));
    g_assert(sed_cell_is_age(cell_0, 33));

    sed_cell_destroy(cell);
    sed_cell_destroy(cell_0);
    sed_column_destroy(s);
}


void
test_sed_cell_add_column_null(void)
{
    Sed_cell cell = sed_cell_new_classed(NULL, 33, S_SED_TYPE_SAND);
    Sed_cell cell_0;

    cell_0 = sed_cell_add_column(cell, NULL);

    g_assert(cell_0 == NULL);

    sed_cell_destroy(cell);
}


void
test_sed_column_add(void)
{
    Sed_column d = sed_column_new(5);
    Sed_column s = sed_column_new(5);
    Sed_cell   c = sed_cell_new_classed(NULL, 58, S_SED_TYPE_SAND | S_SED_TYPE_CLAY);
    Sed_column d_0;
    double mass_in;

    sed_column_set_base_height(s, 666);
    sed_column_set_base_height(d, 868);

    sed_column_add_cell(s, c);

    mass_in = sed_column_mass(s);

    d_0 = sed_column_add(d, s);

    g_assert(sed_column_mass_is(d, sed_column_mass(s)));
    g_assert(sed_column_mass_is(s, mass_in));
    g_assert(d_0 == d);
    g_assert(fabs(sed_column_base_height(d) - 868) < 1e-12);

    sed_column_add(d, s);

    g_assert(sed_column_mass_is(d, 2.*mass_in));

    sed_cell_destroy(c);
    sed_column_destroy(d);
    sed_column_destroy(s);
}


void
test_sed_column_add_dup(void)
{
    Sed_column d;
    Sed_column s   = sed_column_new(5);
    Sed_column s_0 = sed_column_new(5);
    Sed_cell   c = sed_cell_new_classed(NULL, 58, S_SED_TYPE_SAND | S_SED_TYPE_CLAY);

    sed_column_set_base_height(s, 666);
    sed_column_set_z_res(s, 1.2);

    d = sed_column_add(NULL, s);

    g_assert(d != NULL);
    g_assert(d != s);
    g_assert(sed_column_is_same_data(d, s_0));
    g_assert(sed_column_is_empty(d));

    sed_column_add_cell(s, c);
    sed_column_destroy(d);

    d = sed_column_add(NULL, s);

    g_assert(sed_column_mass_is(d, sed_column_mass(s)));

    sed_cell_destroy(c);
    sed_column_destroy(d);
    sed_column_destroy(s);
    sed_column_destroy(s_0);
}


void
test_sed_column_add_null(void)
{
    Sed_column d = sed_column_new(5);

    g_assert(sed_column_add(d, NULL) == NULL);

    sed_column_destroy(d);
}


void
test_sed_column_remove(void)
{
    Sed_column d_0;
    Sed_column d = sed_column_new(5);
    Sed_column s = sed_column_new(5);
    Sed_cell c_1 = sed_cell_new_classed(NULL, 25, S_SED_TYPE_SAND);

    sed_column_set_base_height(d, 100);

    sed_column_add_cell(d, c_1);
    sed_column_add_cell(d, c_1);
    sed_column_add_cell(d, c_1);

    sed_column_set_base_height(s, 125);

    d_0 = sed_column_remove(d, s);

    g_assert(d_0 == d);
    g_assert(sed_column_base_height_is(d, 100));
    g_assert(sed_column_top_height_is(d, 125));
    g_assert(sed_column_mass_is(d, sed_cell_mass(c_1)));

    sed_cell_destroy(c_1);
    sed_column_destroy(d);
    sed_column_destroy(s);
}


void
test_sed_column_remove_above(void)
{
    Sed_column d = sed_column_new(5);
    Sed_column s = sed_column_new(5);
    Sed_cell c_1 = sed_cell_new_classed(NULL, 25, S_SED_TYPE_SAND);

    sed_column_set_base_height(d, 100);

    sed_column_add_cell(d, c_1);

    sed_column_set_base_height(s, 135);

    sed_column_remove(d, s);

    g_assert(sed_column_base_height_is(d, 100));
    g_assert(sed_column_top_height_is(d, 125));
    g_assert(sed_column_mass_is(d, sed_cell_mass(c_1)));

    sed_cell_destroy(c_1);
    sed_column_destroy(d);
    sed_column_destroy(s);
}


void
test_sed_column_remove_below(void)
{
    Sed_column d = sed_column_new(5);
    Sed_column s = sed_column_new(5);
    Sed_cell c_1 = sed_cell_new_classed(NULL, 25, S_SED_TYPE_SAND);

    sed_column_set_base_height(d, 100);

    sed_column_add_cell(d, c_1);

    sed_column_set_base_height(s, 75);

    sed_column_remove(d, s);

    g_assert(sed_column_base_height_is(d, 75));
    g_assert(sed_column_top_height_is(d, 75));
    g_assert(sed_column_is_empty(d));

    sed_cell_destroy(c_1);
    sed_column_destroy(d);
    sed_column_destroy(s);
}


void
test_sed_column_remove_empty(void)
{
    Sed_column d = sed_column_new(5);
    Sed_column s = sed_column_new(55);

    sed_column_set_base_height(d, 100);
    sed_column_set_base_height(s, 125);

    sed_column_remove(d, s);

    g_assert(sed_column_is_empty(d));
    g_assert(sed_column_base_height_is(d, 100));

    sed_column_set_base_height(s, 75);

    sed_column_remove(d, s);

    g_assert(sed_column_is_empty(d));
    g_assert(sed_column_base_height_is(d, 75));

    sed_column_destroy(d);
    sed_column_destroy(s);
}

int
main(int argc, char* argv[])
{
    eh_init_glib();

    if (!sed_test_setup_sediment("sediment")) {
        eh_exit(EXIT_FAILURE);
    }

    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/libsed/sed_column/new", &test_sed_column_new);
    g_test_add_func("/libsed/sed_column/destroy", &test_sed_column_destroy);
    g_test_add_func("/libsed/sed_column/copy", &test_sed_column_copy);
    g_test_add_func("/libsed/sed_column/clear", &test_sed_column_clear);
    g_test_add_func("/libsed/sed_column/stack_cell_loc", &test_sed_column_stack_cells_loc);
    g_test_add_func("/libsed/sed_column/add_cell", &test_sed_column_add_cell);
    g_test_add_func("/libsed/sed_column/add_cell_small", &test_sed_column_add_cell_small);
    g_test_add_func("/libsed/sed_column/add_cell_large", &test_sed_column_add_cell_large);
    g_test_add_func("/libsed/sed_column/stack_cell", &test_sed_column_stack_cell);
    g_test_add_func("/libsed/sed_column/resize_cell", &test_sed_column_resize_cell);
    g_test_add_func("/libsed/sed_column/compact_cell", &test_sed_column_compact_cell);
    g_test_add_func("/libsed/sed_column/height", &test_sed_column_height);
    g_test_add_func("/libsed/sed_column/top_cell", &test_sed_column_top_cell);
    g_test_add_func("/libsed/sed_column/nth_cell", &test_sed_column_nth_cell);
    g_test_add_func("/libsed/sed_column/load", &test_sed_column_load);
    g_test_add_func("/libsed/sed_column/load_at", &test_sed_column_load_at);
    g_test_add_func("/libsed/sed_column/total_load", &test_sed_column_total_load);
    g_test_add_func("/libsed/sed_column/top_index", &test_sed_column_top_index);
    g_test_add_func("/libsed/sed_column/is_valid_index", &test_sed_column_is_valid_index);
    g_test_add_func("/libsed/sed_column/is_get_index", &test_sed_column_is_get_index);
    g_test_add_func("/libsed/sed_column/is_set_index", &test_sed_column_is_set_index);
    g_test_add_func("/libsed/sed_column/index_at", &test_sed_column_index_at);
    g_test_add_func("/libsed/sed_column/index_thickness", &test_sed_column_index_thickness);
    g_test_add_func("/libsed/sed_column/index_depth", &test_sed_column_index_depth);
    g_test_add_func("/libsed/sed_column/depth_age", &test_sed_column_depth_age);
    g_test_add_func("/libsed/sed_column/top_nbins", &test_sed_column_top_nbins);
    g_test_add_func("/libsed/sed_column/rebin", &test_sed_column_rebin);
    g_test_add_func("/libsed/sed_column/thickness_index", &test_sed_column_thickness_index);
    g_test_add_func("/libsed/sed_column/is_above", &test_sed_column_is_above);
    g_test_add_func("/libsed/sed_column/is_below", &test_sed_column_is_below);
    g_test_add_func("/libsed/sed_column/chop", &test_sed_column_chop);
    g_test_add_func("/libsed/sed_column/chomp", &test_sed_column_chomp);
    g_test_add_func("/libsed/sed_column/strip", &test_sed_column_strip);
    g_test_add_func("/libsed/sed_column/add_column", &test_sed_cell_add_column);
    g_test_add_func("/libsed/sed_column/add", &test_sed_column_add);
    g_test_add_func("/libsed/sed_column/remove", &test_sed_column_remove);
    g_test_add_func("/libsed/sed_column/top", &test_sed_column_top);
    g_test_add_func("/libsed/sed_column/top_rho", &test_sed_column_top_rho);
    g_test_add_func("/libsed/sed_column/top_age", &test_sed_column_top_age);
    g_test_add_func("/libsed/sed_column/top_prop", &test_sed_column_top_property);
    g_test_add_func("/libsed/sed_column/top_prop_with_load",
        &test_sed_column_top_property_with_load);

    g_test_add_func("/libsed/sed_column/set/height", &test_sed_column_set_height);
    g_test_add_func("/libsed/sed_column/set/x", &test_sed_column_set_x_position);
    g_test_add_func("/libsed/sed_column/set/y", &test_sed_column_set_y_position);
    g_test_add_func("/libsed/sed_column/set/dz", &test_sed_column_set_z_res);

    g_test_add_func("/libsed/sed_column/limits/destroy_null",
        &test_sed_column_destroy_null);
    g_test_add_func("/libsed/sed_column/limits/copy_null", &test_sed_column_copy_null);
    g_test_add_func("/libsed/sed_column/limits/new_neg", &test_sed_column_new_neg);
    g_test_add_func("/libsed/sed_column/limits/new_zero", &test_sed_column_new_zero);
    g_test_add_func("/libsed/sed_column/limits/add_empty_cell",
        &test_sed_column_add_cell_empty);
    g_test_add_func("/libsed/sed_column/limits/resize_neg",
        &test_sed_column_resize_cell_neg);
    g_test_add_func("/libsed/sed_column/limits/resize_over",
        &test_sed_column_resize_cell_over);
    g_test_add_func("/libsed/sed_column/limits/resize_under",
        &test_sed_column_resize_cell_under);
    g_test_add_func("/libsed/sed_column/limits/load_partial",
        &test_sed_column_load_partial);
    g_test_add_func("/libsed/sed_column/limits/load_null", &test_sed_column_load_null);
    g_test_add_func("/libsed/sed_column/limits/neg_start", &test_sed_column_load_neg_start);
    g_test_add_func("/libsed/sed_column/limits/load_neg_bins",
        &test_sed_column_load_neg_bins);
    g_test_add_func("/libsed/sed_column/limits/top_index_null",
        &test_sed_column_top_index_null);
    g_test_add_func("/libsed/sed_column/limits/top_index_empty",
        &test_sed_column_top_index_empty);
    g_test_add_func("/libsed/sed_column/limits/nth_empty", &test_sed_column_nth_cell_empty);
    g_test_add_func("/libsed/sed_column/limits/nth_null", &test_sed_column_nth_cell_null);
    g_test_add_func("/libsed/sed_column/limits/index_at_base",
        &test_sed_column_index_at_base);
    g_test_add_func("/libsed/sed_column/limits/index_at_above",
        &test_sed_column_index_at_above);
    g_test_add_func("/libsed/sed_column/limits/index_at_below",
        &test_sed_column_index_at_below);
    g_test_add_func("/libsed/sed_column/limits/height_empty",
        &test_sed_column_height_empty);
    g_test_add_func("/libsed/sed_column/limits/height_null", &test_sed_column_height_null);
    g_test_add_func("/libsed/sed_column/limits/thickenss_neg",
        &test_sed_column_thickness_index_neg);
    g_test_add_func("/libsed/sed_column/limits/thickness_above",
        &test_sed_column_thickness_index_above);
    g_test_add_func("/libsed/sed_column/limits/thickness_null",
        &test_sed_column_thickness_index_null);
    g_test_add_func("/libsed/sed_column/limits/thickness_empty",
        &test_sed_column_thickness_index_empty);
    g_test_add_func("/libsed/sed_column/limits/chomp_above", &test_sed_column_chomp_above);
    g_test_add_func("/libsed/sed_column/limits/chomp_below", &test_sed_column_chomp_below);
    g_test_add_func("/libsed/sed_column/limits/chomp_null", &test_sed_column_chomp_null);
    g_test_add_func("/libsed/sed_column/limits/extract_above_0",
        &test_sed_column_extract_above_0);
    g_test_add_func("/libsed/sed_column/limits/extract_above_1",
        &test_sed_column_extract_above_1);
    g_test_add_func("/libsed/sed_column/limits/extract_above_2",
        &test_sed_column_extract_above_2);
    g_test_add_func("/libsed/sed_column/limits/chop_above", &test_sed_column_chop_above);
    g_test_add_func("/libsed/sed_column/limits/chop_below", &test_sed_column_chop_below);
    g_test_add_func("/libsed/sed_column/limits/chop_null", &test_sed_column_chop_null);
    g_test_add_func("/libsed/sed_column/limits/add_cell_null",
        &test_sed_cell_add_column_null);
    g_test_add_func("/libsed/sed_column/limits/add_cell_dup",
        &test_sed_cell_add_column_dup);
    g_test_add_func("/libsed/sed_column/limits/add_null", &test_sed_column_add_null);
    g_test_add_func("/libsed/sed_column/limits/add_dup", &test_sed_column_add_dup);
    g_test_add_func("/libsed/sed_column/limits/remove_above",
        &test_sed_column_remove_above);
    g_test_add_func("/libsed/sed_column/limits/remove_below",
        &test_sed_column_remove_below);
    g_test_add_func("/libsed/sed_column/limits/remove_empty",
        &test_sed_column_remove_empty);
    g_test_add_func("/libsed/sed_column/limits/top_above", &test_sed_column_top_above);
    g_test_add_func("/libsed/sed_column/limits/top_below", &test_sed_column_top_below);
    g_test_add_func("/libsed/sed_column/limits/top_null", &test_sed_column_top_null);
    g_test_add_func("/libsed/sed_column/limits/top_empty", &test_sed_column_top_empty);

    //g_test_add_func ("/libsed/sed_column/io/write", &test_sed_column_write );
    //g_test_add_func ("/libsed/sed_column/io/read", &test_sed_column_read  );


    g_test_run();

    return EXIT_SUCCESS;
}

