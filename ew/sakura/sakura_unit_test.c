#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include <check.h>

#include "sakura_local.h"
#include "sakura.h"

START_TEST(test_sakura_array_new)
{
    Sakura_array* a = NULL;
    gint          i;
    gint          n;

    a = sakura_array_new(10, 5);

    fail_unless(a != NULL, "NULL is not a valid array");

    fail_unless(a->x != NULL, "Member x not set correctly");
    fail_unless(a->w != NULL, "Member w not set correctly");
    fail_unless(a->c != NULL, "Member c not set correctly");
    fail_unless(a->u != NULL, "Member u not set correctly");
    fail_unless(a->h != NULL, "Member h not set correctly");

    fail_unless(a->len    == 10, "Array length set incorrectly");
    fail_unless(a->n_grain == 5, "Number of grain types set incorrectly");

    fail_unless(a->c_grain   != NULL, "Member c_grain not set correctly");
    fail_unless(a->c_grain[0] != NULL, "Member c_grain not set correctly");

    for (i = -2 ; i < 12 ; i++) {
        fail_unless(eh_compare_dbl(a->x[i], 0, 1e-12), "Member x not initialized");
        fail_unless(eh_compare_dbl(a->w[i], 0, 1e-12), "Member w not initialized");
        fail_unless(eh_compare_dbl(a->c[i], 0, 1e-12), "Member c not initialized");
        fail_unless(eh_compare_dbl(a->u[i], 0, 1e-12), "Member u not initialized");
        fail_unless(eh_compare_dbl(a->h[i], 0, 1e-12), "Member h not initialized");
    }

    for (i = -2 ; i < 12 ; i++)
        for (n = 0 ; n < 4 ; n++) {
            fail_unless(eh_compare_dbl(a->c_grain[i][n], 0, 1e-12),
                "Member c_grain not initialized");
        }

    sakura_array_destroy(a);
}
END_TEST

START_TEST(test_sakura_array_copy)
{
    Sakura_array* s = NULL;
    Sakura_array* d = NULL;

    s = sakura_array_new(50, 3);
    d = sakura_array_new(50, 3);

    s->h[16]          = 1973;
    s->c_grain[51][2] = 3.14;
    s->c_grain[-2][0] = -1;

    sakura_array_copy(d, s);

    fail_unless(d != NULL);
    fail_unless(s != d);
    fail_unless(s->x != d->x);
    fail_unless(s->h != d->h);
    fail_unless(s->w != d->w);
    fail_unless(s->c != d->c);
    fail_unless(s->c_grain != d->c_grain);

    fail_unless(d->len    == s->len, "Array lengths should be the same");
    fail_unless(d->n_grain == s->n_grain, "Number of grain sizes not copied correctly");

    fail_unless(eh_compare_dbl(d->h[16], s->h[16], 1e-12));
    fail_unless(eh_compare_dbl(d->c_grain[51][2], s->c_grain[51][2], 1e-12));
    fail_unless(eh_compare_dbl(d->c_grain[-2][0], s->c_grain[-2][0], 1e-12));

    sakura_array_destroy(s);
    sakura_array_destroy(d);
}
END_TEST

START_TEST(test_sakura_array_set)
{
    Sakura_array* s = NULL;
    double*       x;
    gint          i;

    s = sakura_array_new(100, 7);

    x = eh_new(double, 100);

    for (i = 0 ; i < 100 ; i++) {
        x[i] = i;
    }

    sakura_array_set_x(s, x);

    fail_unless(eh_compare_dbl(s->x[0], x[0], 1e-12));
    fail_unless(eh_compare_dbl(s->x[99], x[99], 1e-12));

    fail_unless(eh_compare_dbl(s->x[-1], -1, 1e-12));
    fail_unless(eh_compare_dbl(s->x[-2], -2, 1e-12));

    fail_unless(eh_compare_dbl(s->x[100], 100, 1e-12));
    fail_unless(eh_compare_dbl(s->x[101], 101, 1e-12));

    sakura_array_set_w(s, x);

    fail_unless(eh_compare_dbl(s->w[0], x[0], 1e-12));
    fail_unless(eh_compare_dbl(s->w[99], x[99], 1e-12));

    fail_unless(eh_compare_dbl(s->w[-1], 0, 1e-12));
    fail_unless(eh_compare_dbl(s->w[-2], 0, 1e-12));

    fail_unless(eh_compare_dbl(s->w[100], 99, 1e-12));
    fail_unless(eh_compare_dbl(s->w[101], 99, 1e-12));

    sakura_array_destroy(s);
}
END_TEST

START_TEST(test_sakura_array_set_bc)
{
    Sakura_array* s        = sakura_array_new(128, 5);
    Sakura_node*  left_bc  = sakura_node_new(1, 2, 3, NULL, 5);
    Sakura_node*  right_bc = sakura_node_new(4, 5, 6, NULL, 5);
    gint i, n;

    for (n = 0 ; n < 5 ; n++) {
        left_bc ->c_grain[n] = n;
        right_bc->c_grain[n] = 2 * n;
    }

    sakura_array_set_bc(s, left_bc, right_bc);

    for (i = -2 ; i < 0 ; i++) {
        fail_unless(eh_compare_dbl(s->u[i], 1, 1e-12));
        fail_unless(eh_compare_dbl(s->c[i], 2, 1e-12));
        fail_unless(eh_compare_dbl(s->h[i], 3, 1e-12));

        for (n = 0 ; n < 5 ; n++) {
            fail_unless(eh_compare_dbl(s->c_grain[i][n], n, 1e-12));
        }
    }

    for (i = 128 ; i <= 129 ; i++) {
        fail_unless(eh_compare_dbl(s->u[i], 4, 1e-12));
        fail_unless(eh_compare_dbl(s->c[i], 5, 1e-12));
        fail_unless(eh_compare_dbl(s->h[i], 6, 1e-12));

        for (n = 0 ; n < 5 ; n++) {
            fail_unless(eh_compare_dbl(s->c_grain[i][n], 2 * n, 1e-12));
        }
    }

    sakura_array_destroy(s);
    sakura_node_destroy(left_bc);
    sakura_node_destroy(right_bc);
}
END_TEST

START_TEST(test_sakura_set_outflow)
{
    Sakura_array* s = sakura_array_new(1024, 5);
    double*       x = eh_new(double, 1024);
    Sakura_node*  o = sakura_node_new(1, 2, 3, NULL, 5);
    gint i, n;

    for (i = 0 ; i < 1024 ; i++) {
        x[i] = i;
    }

    sakura_array_set_x(s, x);

    eh_dbl_array_set(s->u - 2, 1028, 9.81);
    eh_dbl_array_set(s->c - 2, 1028, 9.81);
    eh_dbl_array_set(s->h - 2, 1028, 9.81);

    eh_dbl_array_set(o->c_grain, 5, 45);

    sakura_set_outflow(o, s, 512, 1., 2.);

    fail_unless(eh_compare_dbl(o->u, 0., 1e-12));
    fail_unless(eh_compare_dbl(o->c, 0., 1e-12));
    fail_unless(eh_compare_dbl(o->h, 0., 1e-12));

    for (n = 0 ; n < 5 ; n++) {
        fail_unless(eh_compare_dbl(o->c_grain[n], 0., 1e-12));
    }

    s->u[1023] = 3.14; // These are node values
    s->c[1022] = 3.14; // These are midpoint values
    s->h[1022] = 3.14; // These are midpoint values
    eh_dbl_array_set(s->c_grain[1022], 5, 3.14);     // These are midpoint values

    sakura_set_outflow(o, s, 2048, 1., 2.);

    fail_unless(eh_compare_dbl(o->u, 3.14, 1e-12));
    fail_unless(eh_compare_dbl(o->c, 3.14, 1e-12));
    fail_unless(eh_compare_dbl(o->h, 3.14, 1e-12));

    for (n = 0 ; n < 5 ; n++) {
        fail_unless(eh_compare_dbl(o->c_grain[n], 3.14, 1e-12));
    }

    eh_free(x);
    sakura_node_destroy(o);
    sakura_array_destroy(s);
}
END_TEST

START_TEST(test_sakura_get_depth)
{
    Sakura_array*   a    = sakura_array_new(16, 3);
    Sakura_arch_st* data = eh_new(Sakura_arch_st, 1);
    double          s;
    gint            i;

    data->b        = eh_new(Sakura_bathy_st, 1);
    data->b->x     = eh_new(double, 16);
    data->b->depth = eh_new(double, 16);
    data->b->len   = 16;
    data->b->dx    = 1;

    for (i = 0 ; i < 16 ; i++) {
        data->b->x    [i] = i * data->b->dx;
        data->b->depth[i] = data->b->x[i];
        a->x[i]           = data->b->x[i];
    }

    s = -sakura_get_sin_slope(sakura_get_depth, data, a, 4);

    fail_unless(eh_compare_dbl(s, sqrt(2) / 2., 1e-12));

    sakura_array_destroy(a);
}
END_TEST

START_TEST(test_sakura_mid_vel)
{
    Sakura_array* a     = sakura_array_new(2048, 5);
    Sakura_array* a_mid = sakura_array_new(2048, 5);
    Sakura_arch_st* data = eh_new(Sakura_arch_st, 1);
    Sakura_const_st Const;
    gint i;
    gint ind_head;

    data->b        = eh_new(Sakura_bathy_st, 1);
    data->b->x     = eh_new(double, 2048);
    data->b->depth = eh_new(double, 2048);
    data->b->len   = 2048;
    data->b->dx    = 1;

    for (i = 0 ; i < 2048 ; i++) {
        data->b->x    [i] = i * data->b->dx;
        data->b->depth[i] = .001 * data->b->x[i];
    }

    sakura_array_set_x(a, data->b->x);

    Const.dt         = 1.;
    Const.get_depth  = sakura_get_depth;
    Const.depth_data = data;
    Const.c_drag     = .004;
    Const.mu_water   = S_MU_WATER;

    ind_head = 24;

    for (i = -2 ; i <= ind_head ; i++) {
        a->u[i] = 1.;
        a->h[i] = 3.;
        a->c[i] = 32.;
    }

    a_mid->u[-2] = a->u[0];
    a_mid->u[-1] = a->u[0];
    a_mid->u[ 0] = a->u[0];

    calculate_mid_vel(a_mid, a, ind_head, &Const);

    // u[0] is not set as it's the river velocity
    for (i = 1 ; i <= ind_head ; i++) {
        fail_if(eh_compare_dbl(a_mid->u[i], 0, 1e-12));
    }

    sakura_array_destroy(a_mid);
    sakura_array_destroy(a);
}
END_TEST

START_TEST(test_sakura_mid_c_and_h)
{
    Sakura_array* a      = sakura_array_new(2048, 5);
    Sakura_array* a_mid  = sakura_array_new(2048, 5);
    Sakura_array* a_next = sakura_array_new(2048, 5);
    gint          i;

    for (i = -2 ; i < 24 ; i++) {
        a->c[i] = 1;
        a->h[i] = 10;
        a_next->c[i] = 2;
        a_next->h[i] = 20;
    }

    calculate_mid_c_and_h(a_mid, a, a_next);

    for (i = -2 ; i < 24 ; i++) {
        fail_unless(eh_compare_dbl(a_mid->c[i], 1.5, 1e-12));
        fail_unless(eh_compare_dbl(a_mid->h[i], 15., 1e-12));
    }

    sakura_array_destroy(a_next);
    sakura_array_destroy(a_mid);
    sakura_array_destroy(a);
}
END_TEST

START_TEST(test_sakura_next_vel)
{
    Sakura_array* a      = sakura_array_new(2048, 5);
    Sakura_array* a_mid  = sakura_array_new(2048, 5);
    Sakura_array* a_next = sakura_array_new(2048, 5);
    Sakura_arch_st* data = eh_new(Sakura_arch_st, 1);
    Sakura_const_st Const;
    gint i;
    gint ind_head;

    data->b        = eh_new(Sakura_bathy_st, 1);
    data->b->x     = eh_new(double, 2048);
    data->b->depth = eh_new(double, 2048);
    data->b->len   = 2048;
    data->b->dx    = 1;

    for (i = 0 ; i < 2048 ; i++) {
        data->b->x    [i] = i * data->b->dx;
        data->b->depth[i] = .001 * data->b->x[i];
    }

    sakura_array_set_x(a, data->b->x);

    Const.dt         = 1.;
    Const.get_depth  = sakura_get_depth;
    Const.depth_data = data;
    Const.c_drag     = .004;
    Const.mu_water   = S_MU_WATER;

    ind_head = 24;

    for (i = -2 ; i <= ind_head ; i++) {
        a->u[i] = 1.;
        a->h[i] = 3.;
        a->c[i] = 32.;
        a_mid->u[i] = 1.25;
        a_mid->h[i] = 3.01;
        a_mid->c[i] = 32 * .999;

    }

    a_mid->u[-2] = a->u[0];
    a_mid->u[-1] = a->u[0];
    a_mid->u[ 0] = a->u[0];

    calculate_next_vel(a, a_mid, a_next, ind_head, &Const);

    for (i = 1 ; i <= ind_head ; i++) {
        fail_if(eh_compare_dbl(a_next->u[i], a->u[i], 1e-12));
        fail_if(eh_compare_dbl(a_next->u[i], a_mid->u[i], 1e-12));
    }

    sakura_array_destroy(a_next);
    sakura_array_destroy(a_mid);
    sakura_array_destroy(a);
}
END_TEST

START_TEST(test_sakura_next_c_and_h)
{
    Sakura_array* a      = sakura_array_new(2048, 5);
    Sakura_array* a_mid  = sakura_array_new(2048, 5);
    Sakura_array* a_next = sakura_array_new(2048, 5);
    Sakura_arch_st* data = eh_new(Sakura_arch_st, 1);
    Sakura_sediment* sed = sakura_sediment_new(5);
    Sakura_const_st Const;
    gint i, n;
    gint ind_head;
    double phe_bottom[5] = { .2, .2, .2, .2, .2 };

    {
        double rho_dep   [5] = { 1850, 1600, 1400, 1300, 1200 };
        double rho_grain [5] = { 2650, 2650, 2650, 2650, 2650 };
        double equiv_dia [5] = { 202, 105, 69, 25, 10   };
        double u_settling[5];

        sakura_sediment_set_rho_dep(sed, rho_dep);
        sakura_sediment_set_rho_grain(sed, rho_grain);

        eh_dbl_array_mult(equiv_dia, 5, 1e-5);

        for (i = 0 ; i < 5 ; i++) {
            u_settling[i] = sakura_settling_velocity(rho_grain[i], equiv_dia[i], S_RHO_WATER,
                    S_MU_WATER);
        }

        sakura_sediment_set_u_settling(sed, u_settling);
    }

    data->b        = eh_new(Sakura_bathy_st, 1);
    data->b->x     = eh_new(double, 2048);
    data->b->width = eh_new(double, 2048);
    data->b->depth = eh_new(double, 2048);
    data->b->len   = 2048;
    data->b->dx    = 1;
    data->phe      = phe_bottom;
    data->n_grains = 5;

    for (i = 0 ; i < 2048 ; i++) {
        data->b->x    [i] = i * data->b->dx;
        data->b->width[i] = 100.;
        data->b->depth[i] = .001 * data->b->x[i];
    }

    sakura_array_set_x(a, data->b->x);
    sakura_array_set_w(a, data->b->width);
    sakura_array_set_x(a_mid, data->b->x);
    sakura_array_set_w(a_mid, data->b->width);
    sakura_array_set_x(a_next, data->b->x);
    sakura_array_set_w(a_next, data->b->width);

    Const.dt         = 1.;
    Const.e_a        = .00153;
    Const.e_b        = .0204;
    Const.sua        = 30;
    Const.sub        = .5e3;
    Const.c_drag     = .004;
    Const.dep_start  = 1000;
    Const.mu_water      = S_MU_WATER;
    Const.rho_sea_water = S_RHO_SEA_WATER;
    Const.get_depth  = sakura_get_depth;
    Const.add        = sakura_add;
    Const.remove     = sakura_remove;
    Const.get_phe    = sakura_get_phe;
    Const.depth_data   = data;
    Const.add_data     = data;
    Const.remove_data  = data;
    Const.get_phe_data = data;

    ind_head = 24;

    for (i = -2 ; i <= ind_head ; i++) {
        a->u[i] = 1.;
        a->h[i] = 3.;
        a->c[i] = 32.;

        for (n = 0 ; n < 5 ; n++) {
            a->c_grain[i][n] = .2 * a->c[i];
        }

        a_mid->u[i] = 1.25;
    }

    a_mid->u[-2] = a->u[0];
    a_mid->u[-1] = a->u[0];
    a_mid->u[ 0] = a->u[0];
    a_mid->u[ind_head] = 1.3;

    calculate_next_c_and_h(a_next, a, a_mid->u, ind_head, &Const, sed);

    for (i = 1 ; i <= ind_head ; i++) {
        fail_if(eh_compare_dbl(a_next->h[i], a->h[i], 1e-12));
        fail_if(eh_compare_dbl(a_next->c[i], a->c[i], 1e-12));
    }

    sakura_sediment_destroy(sed);

    sakura_array_destroy(a_next);
    sakura_array_destroy(a_mid);
    sakura_array_destroy(a);
}
END_TEST

START_TEST(test_sakura_head_index)
{
    Sakura_array* a = sakura_array_new(2048, 5);
    double*       u = eh_new(double, 2048) + 2;
    gint   i;
    gint   ind_head;
    double x_head;
    double dx = 1.;
    double dt = 1.;

    ind_head = 24;
    x_head   = 24.2;

    for (i = -2 ; i <= ind_head ; i++) {
        u[i]    = 1.25;
        a->u[i] = 1.;
        a->h[i] = 3.;
        a->c[i] = 32.;
    }

    ind_head = calculate_head_index(a, u, ind_head, dx, dt, &x_head);

    fail_unless(ind_head == 25);
    fail_unless(eh_compare_dbl(x_head, 24.2 + 1.25, 1e-12));

    u -= 2;

    eh_free(u);
    sakura_array_destroy(a);

}
END_TEST

START_TEST(test_sakura_mass_in_susp)
{
    Sakura_array*    a = sakura_array_new(2048, 5);
    Sakura_sediment* s = sakura_sediment_new(5);
    double mass;
    double node_mass;
    gint   i;

    {
        double rho_dep   [5] = { 1850, 1600, 1400, 1300, 1200 };
        double rho_grain [5] = { 2650, 2650, 2650, 2650, 2650 };
        double equiv_dia [5] = { 202, 105, 69, 25, 10   };
        double u_settling[5];

        sakura_sediment_set_rho_dep(s, rho_dep);
        sakura_sediment_set_rho_grain(s, rho_grain);

        eh_dbl_array_mult(equiv_dia, 5, 1e-5);

        for (i = 0 ; i < 5 ; i++) {
            u_settling[i] = sakura_settling_velocity(rho_grain[i], equiv_dia[i], S_RHO_WATER,
                    S_MU_WATER);
        }

        sakura_sediment_set_u_settling(s, u_settling);
    }

    {
        gint   n;
        double node_dx = 1;
        double node_w  = 100;
        double node_h  = 10;
        double node_c  = 2;

        for (i = -2 ; i < 2050 ; i++) {
            a->x[i] = i * node_dx;
            a->w[i] = node_w;
            a->h[i] = node_h;

            for (n = 0 ; n < 5 ; n++) {
                a->c_grain[i][n] = node_c;
            }
        }

        mass = sakura_array_mass_in_susp(a, s);

        for (n = 0, node_mass = 0 ; n < 5 ; n++) {
            node_mass += node_w * node_h * node_dx * node_c * s->rho_grain[n];
        }
    }

    fail_unless(eh_compare_dbl(mass, node_mass * 2048, 1e-12));

}
END_TEST

Suite*
sakura_suite(void)
{
    Suite* s = suite_create("Sakura");
    TCase* test_case_core = tcase_create("Core");

    suite_add_tcase(s, test_case_core);

    tcase_add_test(test_case_core, test_sakura_array_new);
    tcase_add_test(test_case_core, test_sakura_array_copy);
    tcase_add_test(test_case_core, test_sakura_array_set);
    tcase_add_test(test_case_core, test_sakura_array_set_bc);
    tcase_add_test(test_case_core, test_sakura_set_outflow);
    tcase_add_test(test_case_core, test_sakura_get_depth);
    tcase_add_test(test_case_core, test_sakura_mid_vel);
    tcase_add_test(test_case_core, test_sakura_next_c_and_h);
    tcase_add_test(test_case_core, test_sakura_mid_c_and_h);
    tcase_add_test(test_case_core, test_sakura_next_vel);
    tcase_add_test(test_case_core, test_sakura_head_index);
    tcase_add_test(test_case_core, test_sakura_mass_in_susp);

    return s;
}

int
main(void)
{
    int n;
    GError*      error = NULL;

    eh_init_glib();

    {
        Suite* s = sakura_suite();
        SRunner* sr = srunner_create(s);

        srunner_run_all(sr, CK_NORMAL);
        n = srunner_ntests_failed(sr);
        srunner_free(sr);
    }

    sed_sediment_unset_env();

    return n;
}


