#include <eh_utils.h>
#include <glib.h>

void
test_create_grid(void)
{
    Eh_dbl_grid g = eh_grid_new(double, 50, 250);

    g_assert(g != NULL);
    g_assert_cmpint(eh_grid_n_x(g), ==, 50);
    g_assert_cmpint(eh_grid_n_y(g), ==, 250);

    g = eh_grid_destroy(g, TRUE);
}


void
test_destroy_grid(void)
{
    Eh_dbl_grid g = eh_grid_new(double, 1, 1);

    g = eh_grid_destroy(g, TRUE);

    g_assert(g == NULL);
}


void
test_cmp_grid(void)
{
    gboolean ok;
    Eh_dbl_grid g_1 = eh_grid_new(double, 50, 250);
    Eh_dbl_grid g_2;

    eh_dbl_grid_randomize(g_1);
    g_2 = eh_grid_dup(g_1);

    g_assert(eh_grid_cmp_x_data(g_1, g_2));
    g_assert(eh_grid_cmp_y_data(g_1, g_2));
    g_assert(eh_grid_cmp_data(g_1, g_2));

    eh_dbl_grid_set_val(g_2, 0, 0, eh_dbl_grid_val(g_2, 0, 0) + 1e-12);
    g_assert(eh_dbl_grid_cmp(g_1, g_2, 1e-10));

    g_assert(!eh_dbl_grid_cmp(g_1, g_2, -1));

    g_1 = eh_grid_destroy(g_1, TRUE);
    g_2 = eh_grid_destroy(g_2, TRUE);

    g_assert(g_1 == NULL);
    g_assert(g_2 == NULL);
}


void
test_cmp_unequal_grid(void)
{
    gboolean ok;
    Eh_dbl_grid g_1 = eh_grid_new(double, 50, 250);
    Eh_dbl_grid g_2 = eh_grid_new(double, 50, 50);

    g_assert(!eh_dbl_grid_cmp(g_1, g_2, -1));

    g_1 = eh_grid_destroy(g_1, TRUE);
    g_2 = eh_grid_destroy(g_2, TRUE);
}


void
test_zero_create_grid(void)
{
    Eh_dbl_grid g;

    g = eh_grid_new(double, 0, 0);

    g_assert(g);

    g_assert_cmpint(eh_grid_n_x(g), ==, 0);
    g_assert_cmpint(eh_grid_n_y(g), ==, 0);
    g_assert(eh_grid_data(g) == NULL);
    g_assert(eh_grid_x(g) == NULL);
    g_assert(eh_grid_y(g) == NULL);
    eh_grid_destroy(g, TRUE);

    g = eh_grid_new(double, 0, 250);
    g_assert(g);
    g_assert_cmpint(eh_grid_n_x(g), ==, 0);
    g_assert_cmpint(eh_grid_n_y(g), ==, 250);
    g_assert(eh_grid_data(g) == NULL);
    g_assert(eh_grid_x(g) == NULL);
    g_assert(eh_grid_y(g) != NULL);
    eh_grid_destroy(g, TRUE);

    g = eh_grid_new(double, 50, 0);
    g_assert(g);
    g_assert_cmpint(eh_grid_n_x(g), ==, 50);
    g_assert_cmpint(eh_grid_n_y(g), ==, 0);
    g_assert(eh_grid_data(g) == NULL);
    g_assert(eh_grid_x(g) != NULL);
    g_assert(eh_grid_y(g) == NULL);
    eh_grid_destroy(g, TRUE);
}


void
test_negative_create_grid(void)
{
    Eh_dbl_grid g;

    g = eh_grid_new(double, -1, 250);
    g_assert(g == NULL);

    g = eh_grid_new(double, 50, -1);
    g_assert(g == NULL);
}


void
test_set_grid(void)
{
    const int nx = g_test_rand_int_range(50, 500);
    const int ny = g_test_rand_int_range(50, 500);
    Eh_dbl_grid g = eh_grid_new(double, nx, ny);
    double total;

    eh_dbl_grid_set(g, 1);
    total = eh_dbl_grid_sum(g);

    g_assert(eh_compare_dbl(total, nx * ny, 1e-16));

    g = eh_grid_destroy(g, TRUE);
}


void
test_dup_grid(void)
{
    const int nx = g_test_rand_int_range(50, 500);
    const int ny = g_test_rand_int_range(50, 500);
    Eh_dbl_grid g = eh_grid_new(double, nx, ny);
    Eh_dbl_grid h;

    g_assert(g);

    eh_dbl_grid_randomize(g);

    h = eh_grid_dup(g);

    g_assert(h);
    g_assert(g != h);
    g_assert(eh_dbl_grid_cmp(g, h, 1e-6));

    eh_grid_destroy(g, TRUE);
    eh_grid_destroy(h, TRUE);
}


void
test_copy_grid(void)
{
    const int nx = g_test_rand_int_range(10, 100);
    const int ny = g_test_rand_int_range(100, 500);
    Eh_dbl_grid src  = eh_grid_new(double, nx, ny);
    Eh_dbl_grid dest = eh_grid_new(double, nx, ny);

    g_assert(src);
    g_assert(dest);

    eh_dbl_grid_randomize(src);

    eh_grid_copy(dest, src);

    g_assert(dest != src);
    g_assert(eh_grid_cmp_data(dest, src));
    g_assert(eh_grid_cmp_x_data(dest, src));
    g_assert(eh_grid_cmp_y_data(dest, src));

    src  = eh_grid_destroy(src, TRUE);
    dest = eh_grid_destroy(dest, TRUE);
}


void
test_sum_grid(void)
{
    const int nx = g_test_rand_int_range(100, 500);
    const int ny = g_test_rand_int_range(10, 100);
    Eh_dbl_grid g = eh_grid_new(double, nx, ny);
    double total;

    g_assert(g);

    eh_dbl_grid_set(g, 1.);

    total = eh_dbl_grid_sum(g);

    g_assert(eh_compare_dbl(total, eh_grid_n_el(g), 1e-12));

    g = eh_grid_destroy(g, TRUE);
}


void
test_bad_sum_grid(void)
{
    const int nx = g_test_rand_int_range(100, 500);
    const int ny = g_test_rand_int_range(10, 100);
    Eh_dbl_grid g = eh_grid_new(double, nx, ny);
    int i, j;
    int n_bad_vals;
    double total;

    n_bad_vals = 0;

    for (i = 0; i < nx ; i++)
        for (j = 0; j < ny; j++) {
            if (g_random_boolean()) {
                eh_dbl_grid_set_val(g, i, j, 1.);
            } else {
                eh_dbl_grid_set_val(g, i, j, -1. + 1e-16);
                n_bad_vals++;
            }
        }

    total = eh_dbl_grid_sum_bad_val(g, -1);
    g_assert(eh_compare_dbl(total, nx * ny - n_bad_vals, 1e-12));

    n_bad_vals = 0;

    for (i = 0 ; i < nx ; i++)
        for (j = 0 ; j < ny ; j++) {
            if (g_random_boolean()) {
                eh_dbl_grid_set_val(g, i, j, 1.);
            } else {
                eh_dbl_grid_set_val(g, i, j, eh_nan());
                n_bad_vals++;
            }
        }

    total = eh_dbl_grid_sum(g);
    g_assert(eh_compare_dbl(total, nx * ny - n_bad_vals, 1e-12));

    g = eh_grid_destroy(g, TRUE);
}

void
test_scalar_mult_grid(void)
{
    const int nx = g_test_rand_int_range(1, 5);
    const int ny = g_test_rand_int_range(100, 500);
    Eh_dbl_grid g = eh_grid_new(double, nx, ny);
    double total_before;
    double total_after;
    double multiplier;

    multiplier = g_random_double();

    eh_dbl_grid_randomize(g);
    total_before = eh_dbl_grid_sum(g);

    eh_dbl_grid_scalar_mult(g, multiplier);
    total_after = eh_dbl_grid_sum(g);

    g_assert(eh_compare_dbl(total_before, total_after / multiplier, 1e-12));

    g = eh_grid_destroy(g, TRUE);
}


void
test_reindex_grid(void)
{
    const int nx = g_test_rand_int_range(100, 500);
    const int ny = g_test_rand_int_range(1, 5);
    Eh_int_grid g = eh_grid_new(int, nx, ny);

    eh_int_grid_set_val(g, 0, 0, 1);
    eh_int_grid_set_val(g, eh_grid_n_x(g) - 1,
        eh_grid_n_y(g) - 1,
        eh_grid_n_el(g));

    eh_grid_x(g)[0]                = 1;
    eh_grid_x(g)[eh_grid_n_x(g) - 1] = eh_grid_n_x(g);

    eh_grid_y(g)[0]                = 1;
    eh_grid_y(g)[eh_grid_n_y(g) - 1] = eh_grid_n_y(g);

    eh_grid_reindex(g, -5, -10);

    g_assert_cmpint(eh_int_grid_val(g, -5, -10), ==, 1);
    g_assert_cmpint(eh_grid_x(g)[-5], ==, 1);
    g_assert_cmpint(eh_grid_y(g)[-10], ==, 1);

    {
        gssize i = -5  + eh_grid_n_x(g) - 1;
        gssize j = -10 + eh_grid_n_y(g) - 1;

        g_assert_cmpint(eh_int_grid_val(g, i, j), ==, eh_grid_n_el(g));
        g_assert(eh_compare_dbl(eh_grid_x(g)[i], eh_grid_n_x(g), 1e-12));
        g_assert(eh_compare_dbl(eh_grid_y(g)[j], eh_grid_n_y(g), 1e-12));
    }

    g = eh_grid_destroy(g, TRUE);
}


void
test_reduce_grid(void)
{
    const int nx = 1;
    const int ny = g_test_rand_int_range(50, 100);
    Eh_dbl_grid g = eh_grid_new(double, nx, ny);
    double initial_sum, final_sum;
    Eh_dbl_grid small_g;

    g_assert(g);

    g = eh_dbl_grid_randomize(g);
    g = eh_dbl_grid_set(g, 1.);

    initial_sum = eh_dbl_grid_sum(g);
    g_assert(initial_sum > 0);

    small_g   = eh_dbl_grid_reduce(g, 1, 10);
    final_sum = eh_dbl_grid_sum(small_g);

    g_assert(small_g);
    g_assert_cmpint(eh_grid_n_x(small_g), ==, nx);
    g_assert_cmpint(eh_grid_n_x(small_g), ==, nx);
    g_assert_cmpint(eh_grid_n_y(small_g), ==, 10);

    g_assert(final_sum > 0);

    //eh_dbl_grid_fprintf( stderr , "%f " , small_g );
    g_assert(eh_compare_dbl(initial_sum, final_sum * ny / 10., 1e-12));

    g       = eh_grid_destroy(g, TRUE);
    small_g = eh_grid_destroy(small_g, TRUE);
}


void
test_rebin_grid(void)
{
    Eh_dbl_grid x, x_new;
    double sum, sum_new;
    gssize i, j;

    x     = eh_grid_new(double, 93, 109);
    x_new = eh_grid_new(double,  2, 1000);

    for (i = 0 ; i < eh_grid_n_x(x) ; i++) {
        eh_grid_x(x)[i] = 33 * i;
    }

    for (j = 0 ; j < eh_grid_n_y(x) ; j++) {
        eh_grid_y(x)[j] = 300 * j;
    }

    eh_dbl_grid_set(x, 1);

    for (i = 0 ; i < eh_grid_n_x(x_new) ; i++) {
        eh_grid_x(x_new)[i] = 10000 * i;
    }

    for (j = 0 ; j < eh_grid_n_y(x_new) ; j++) {
        eh_grid_y(x_new)[j] = 500 * j;
    }

    eh_dbl_grid_rebin_bad_val(x, x_new, 0);

    sum     = eh_dbl_grid_sum(x) * 33 * 300;
    sum_new = eh_dbl_grid_sum(x_new) * 10000 * 500;
    /*
       eh_dbl_grid_fprintf( stderr , "%f " , x_new );
       eh_watch_dbl( sum );
       eh_watch_dbl( sum_new );
    */

    //g_assert (fabs(sum_new-sum)/sum < 1e-2 , "Grid not rebinned correctly" );
    g_assert(eh_compare_dbl(sum_new, sum, 1e-2));

    eh_grid_destroy(x, TRUE);
    eh_grid_destroy(x_new, TRUE);
}


void
test_line_path(void)
{
    Eh_grid g = eh_grid_new(double, 10, 10);
    gssize* path;

    {
        path = eh_dbl_grid_line_ids(g, 2, 2, 2, 2);
        g_assert(path == NULL);
    }

    {
        gssize ans[7] = { 0, 1, 2, 3, 4, 5, -1 };
        path = eh_dbl_grid_line_ids(g, 0, 0, 0, 5);

        g_assert(path != NULL);
        g_assert(eh_grid_path_is_same(path, ans));

        eh_free(path);
    }

    {
        gssize ans[7] = { 0, 11, 22, 33, 44, 55, -1 };
        path = eh_dbl_grid_line_ids(g, 0, 0, 5, 5);

        g_assert(path != NULL);
        g_assert(eh_grid_path_is_same(path, ans));

        eh_free(path);
    }

    {
        gssize ans[7] = { 0, 10, 20, 30, 40, 50, -1 };
        path = eh_dbl_grid_line_ids(g, 0, 0, 5, 0);

        g_assert(path != NULL);
        g_assert(eh_grid_path_is_same(path, ans));

        eh_free(path);
    }

    {
        gssize ans[7] = { 5, 4, 3, 2, 1, 0, -1 };
        path = eh_dbl_grid_line_ids(g, 0, 5, 0, 0);

        g_assert(path != NULL);
        g_assert(eh_grid_path_is_same(path, ans));

        eh_free(path);
    }

    {
        gssize ans[7] = { 50, 40, 30, 20, 10, 0, -1 };
        path = eh_dbl_grid_line_ids(g, 5, 0, 0, 0);

        g_assert(path != NULL);
        g_assert(eh_grid_path_is_same(path, ans));

        eh_free(path);
    }

    {
        gssize ans[10] = { 0, 10, 21, 31, 42, 53, -1 };
        path = eh_dbl_grid_line_ids(g, 0, 0, 5, 3);

        g_assert(path != NULL);
        g_assert(eh_grid_path_is_same(path, ans));

        eh_free(path);
    }

    eh_grid_destroy(g, TRUE);
}

int
main(int argc, char* argv[])
{
    eh_init_glib();

    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/utils/grid/create", &test_create_grid);
    g_test_add_func("/utils/grid/create_zero", &test_zero_create_grid);
    g_test_add_func("/utils/grid/create_neg", &test_negative_create_grid);
    g_test_add_func("/utils/grid/set", &test_set_grid);
    g_test_add_func("/utils/grid/destroy", &test_destroy_grid);

    g_test_add_func("/utils/grid/dup", &test_dup_grid);
    g_test_add_func("/utils/grid/copy", &test_copy_grid);
    g_test_add_func("/utils/grid/sum", &test_sum_grid);
    g_test_add_func("/utils/grid/sum_bad", &test_bad_sum_grid);
    g_test_add_func("/utils/grid/mult", &test_scalar_mult_grid);
    g_test_add_func("/utils/grid/cmp_same", &test_cmp_grid);
    g_test_add_func("/utils/grid/cmp_unequal", &test_cmp_unequal_grid);
    g_test_add_func("/utils/grid/reindex", &test_reindex_grid);
    g_test_add_func("/utils/grid/reduce", &test_reduce_grid);
    g_test_add_func("/utils/grid/rebin", &test_rebin_grid);

    g_test_run();
}

