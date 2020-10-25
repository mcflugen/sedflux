#include <eh_utils.h>
#include <check.h>

START_TEST(test_create_input_val)
{
    Eh_input_val v;

    v = eh_input_val_new();

    fail_if(v == NULL, "Bad input_val handle");

    v = eh_input_val_destroy(v);

    fail_unless(v == NULL, "Destroyed input_val should be set to NULL");

}
END_TEST

START_TEST(test_set_input_val)
{
    Eh_input_val v;

    {
        GError* err = NULL;

        v = eh_input_val_set("-.5", &err);

        if (err != NULL) {
            fprintf(stderr, "%s", err->message);
        }

        fail_unless(eh_input_val_eval(v) == -.5, "Scalar value set incorrectly");
        v = eh_input_val_destroy(v);

        g_clear_error(&err);
    }

    {
        gssize n_evals = 100000;
        gssize i;
        double* vals = eh_new(double, n_evals);
        double mean, var;
        GError* err = NULL;

        v = eh_input_val_set("normal=-.5,.1", &err);

        if (err != NULL) {
            fprintf(stderr, "%s", err->message);
        }

        for (i = 0 ; i < n_evals ; i++) {
            vals[i] = eh_input_val_eval(v);
        }

        mean = eh_dbl_array_mean(vals, n_evals);
        var  = eh_dbl_array_var(vals, n_evals);

        fail_unless(eh_compare_dbl(mean, -.5, .01), "Normal distribution mean set incorrectly");
        fail_unless(eh_compare_dbl(sqrt(var), .1, .01),
            "Normal distribution variance set incorrectly");

        eh_free(vals);
        v = eh_input_val_destroy(v);

        g_clear_error(&err);
    }

    /*
       {
          v = eh_input_val_set( "user=user_test.txt" );

          fail_if( v==NULL , "Could not read user file" );

          v = eh_input_val_destroy( v );
       }
    */
}
END_TEST

Suite*
eh_input_val_suite(void)
{
    Suite* s = suite_create("Eh_input_val");
    TCase* test_case_core = tcase_create("Core");

    suite_add_tcase(s, test_case_core);

    tcase_add_test(test_case_core, test_create_input_val);
    tcase_add_test(test_case_core, test_set_input_val);

    return s;
}

int
test_input_val(void)
{
    int n;

    {
        Suite* s = eh_input_val_suite();
        SRunner* sr = srunner_create(s);

        srunner_run_all(sr, CK_NORMAL);
        n = srunner_ntests_failed(sr);
        srunner_free(sr);
    }

    return n;
}

