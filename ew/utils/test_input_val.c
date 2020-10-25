#include <stdio.h>
#include <glib.h>
#include "utils/utils.h"
#include <eh_utils.h>

void
test_create_input_val(void)
{
    Eh_input_val v;

    v = eh_input_val_new();
    g_assert(v != NULL);

    v = eh_input_val_destroy(v);
    g_assert(v == NULL);

}

void
test_set_input_val(void)
{
    Eh_input_val v;

    {
        GError* err = NULL;

        v = eh_input_val_set("-.5", &err);

        g_assert(v || err != NULL);
        eh_print_on_error(err, "test_set_input_val");
        g_assert(err == NULL);

        g_assert(eh_compare_dbl(eh_input_val_eval(v), -.5, 1e-12));
        v = eh_input_val_destroy(v);
        g_assert(v == NULL);

        g_clear_error(&err);
    }

    {
        const gint n_evals = 100000;
        gint i;
        double* vals = eh_new(double, n_evals);
        double mean, var;
        GError* err = NULL;

        v = eh_input_val_set("normal=-.5,.1", &err);

        g_assert(v || err != NULL);
        eh_print_on_error(err, "test_set_input_val");
        g_assert(err == NULL);

        for (i = 0; i < n_evals; i++) {
            vals[i] = eh_input_val_eval(v);
        }

        mean = eh_dbl_array_mean(vals, n_evals);
        var  = eh_dbl_array_var(vals, n_evals);

        g_assert(eh_compare_dbl(mean, -.5, .01));
        g_assert(eh_compare_dbl(sqrt(var), .1, .01));

        eh_free(vals);
        v = eh_input_val_destroy(v);
        g_assert(v == NULL);

        g_clear_error(&err);
    }
}

int
main(int argc, char* argv[])
{
    eh_init_glib();

    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/utils/input_val/create", &test_create_input_val);
    g_test_add_func("/utils/input_val/set", &test_set_input_val);

    g_test_run();
}

