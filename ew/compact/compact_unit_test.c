#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include <check.h>

int
compact(Sed_column);

START_TEST(test_compact_0)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell;

    {
        double* f = eh_new0(double, sed_sediment_env_n_types());

        f[0] = 1.;
        cell = sed_cell_new_sized(sed_sediment_env_n_types(), 30000, f);

        eh_free(f);
    }

    sed_column_set_sea_level(c, 10);
    sed_column_set_base_height(c, -40000);

    sed_column_add_cell(c, cell);

    {
        double mass_out;
        double mass_in = sed_column_sediment_mass(c);

        compact(c);

        mass_out = sed_column_sediment_mass(c);

        fail_unless(eh_compare_dbl(mass_in, mass_out, 1e-12),
            "Mass balance error in compaction");
    }

    {
        Sed_cell cell_0 = sed_column_nth_cell(c, 0);
        double e_min  = sed_cell_void_ratio_min(cell_0);
        double e      = sed_cell_void_ratio(cell_0);

        fail_unless(eh_compare_dbl(e_min, e, 1e-6), "Cell did not reach maximum compaction");
    }

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}
END_TEST

START_TEST(test_compact_1)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell;

    {
        double* f = eh_new0(double, sed_sediment_env_n_types());

        f[1] = 1.;
        cell = sed_cell_new_sized(sed_sediment_env_n_types(), 30000, f);

        eh_free(f);
    }

    sed_column_set_sea_level(c, 10);
    sed_column_set_base_height(c, -40000);

    sed_column_add_cell(c, cell);

    {
        double mass_out;
        double mass_in = sed_column_sediment_mass(c);

        compact(c);

        mass_out = sed_column_sediment_mass(c);

        fail_unless(eh_compare_dbl(mass_in, mass_out, 1e-12),
            "Mass balance error in compaction");
    }

    {
        Sed_cell cell_0 = sed_column_nth_cell(c, 0);
        double e_min  = sed_cell_void_ratio_min(cell_0);
        double e      = sed_cell_void_ratio(cell_0);

        fail_unless(eh_compare_dbl(e_min, e, 1e-6), "Cell did not reach maximum compaction");
    }

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}
END_TEST

START_TEST(test_compact_2)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell;

    {
        double* f = eh_new0(double, sed_sediment_env_n_types());

        f[2] = 1.;
        cell = sed_cell_new_sized(sed_sediment_env_n_types(), 30000, f);

        eh_free(f);
    }

    sed_column_set_sea_level(c, 10);
    sed_column_set_base_height(c, -40000);

    sed_column_add_cell(c, cell);

    {
        double mass_out;
        double mass_in = sed_column_sediment_mass(c);

        compact(c);

        mass_out = sed_column_sediment_mass(c);

        fail_unless(eh_compare_dbl(mass_in, mass_out, 1e-12),
            "Mass balance error in compaction");
    }

    {
        Sed_cell cell_0 = sed_column_nth_cell(c, 0);
        double e_min  = sed_cell_void_ratio_min(cell_0);
        double e      = sed_cell_void_ratio(cell_0);

        fail_unless(eh_compare_dbl(e_min, e, 1e-6), "Cell did not reach maximum compaction");
    }

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}
END_TEST

START_TEST(test_compact_3)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell;

    {
        double* f = eh_new0(double, sed_sediment_env_n_types());

        f[3] = 1.;
        cell = sed_cell_new_sized(sed_sediment_env_n_types(), 30000, f);

        eh_free(f);
    }

    sed_column_set_sea_level(c, 10);
    sed_column_set_base_height(c, -40000);

    sed_column_add_cell(c, cell);

    {
        double mass_out;
        double mass_in = sed_column_sediment_mass(c);

        compact(c);

        mass_out = sed_column_sediment_mass(c);

        fail_unless(eh_compare_dbl(mass_in, mass_out, 1e-12),
            "Mass balance error in compaction");
    }

    {
        Sed_cell cell_0 = sed_column_nth_cell(c, 0);
        double e_min  = sed_cell_void_ratio_min(cell_0);
        double e      = sed_cell_void_ratio(cell_0);

        fail_unless(eh_compare_dbl(e_min, e, 1e-6), "Cell did not reach maximum compaction");
    }

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}
END_TEST

START_TEST(test_compact_4)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell;

    {
        double* f = eh_new0(double, sed_sediment_env_n_types());

        f[4] = 1.;
        cell = sed_cell_new_sized(sed_sediment_env_n_types(), 30000, f);

        eh_free(f);
    }

    sed_column_set_sea_level(c, 10);
    sed_column_set_base_height(c, -40000);

    sed_column_add_cell(c, cell);

    {
        double mass_out;
        double mass_in = sed_column_sediment_mass(c);

        compact(c);

        mass_out = sed_column_sediment_mass(c);

        fail_unless(eh_compare_dbl(mass_in, mass_out, 1e-12),
            "Mass balance error in compaction");
    }

    {
        Sed_cell cell_0 = sed_column_nth_cell(c, 0);
        double e_min  = sed_cell_void_ratio_min(cell_0);
        double e      = sed_cell_void_ratio(cell_0);

        fail_unless(eh_compare_dbl(e_min, e, 1e-6), "Cell did not reach maximum compaction");
    }

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}
END_TEST

START_TEST(test_compact_mixed)
{
    Sed_column c = sed_column_new(5);
    Sed_cell cell;

    {
        double* f = eh_new0(double, sed_sediment_env_n_types());
        gssize n;

        for (n = 0 ; n < sed_sediment_env_n_types() ; n++) {
            f[n] = 1. / (double)sed_sediment_env_n_types();
        }

        cell = sed_cell_new_sized(sed_sediment_env_n_types(), 30000, f);

        eh_free(f);
    }

    sed_column_set_sea_level(c, 10);
    sed_column_set_base_height(c, -40000);

    sed_column_add_cell(c, cell);

    {
        double mass_out;
        double mass_in = sed_column_sediment_mass(c);

        compact(c);

        mass_out = sed_column_sediment_mass(c);

        fail_unless(eh_compare_dbl(mass_in, mass_out, 1e-12),
            "Mass balance error in compaction");
    }

    {
        Sed_cell cell_0 = sed_column_nth_cell(c, 0);
        double e_min  = sed_cell_void_ratio_min(cell_0);
        double e      = sed_cell_void_ratio(cell_0);

        fail_unless(eh_compare_dbl(e_min, e, 1e-5), "Cell did not reach maximum compaction");
    }

    sed_cell_destroy(cell);
    sed_column_destroy(c);
}
END_TEST

Suite*
sed_compact_suite(void)
{
    Suite* s = suite_create("Compact");
    TCase* test_case_core = tcase_create("Core");

    suite_add_tcase(s, test_case_core);

    tcase_add_test(test_case_core, test_compact_0);
    tcase_add_test(test_case_core, test_compact_1);
    tcase_add_test(test_case_core, test_compact_2);
    tcase_add_test(test_case_core, test_compact_3);
    tcase_add_test(test_case_core, test_compact_4);
    tcase_add_test(test_case_core, test_compact_mixed);

    return s;
}

int
main(void)
{
    int n;
    Sed_sediment sed   = NULL;
    GError*      error = NULL;

    eh_init_glib();

    sed = sed_sediment_scan(SED_SEDIMENT_TEST_FILE, &error);

    if (!sed) {
        eh_error("%s: Unable to read sediment file: %s", SED_SEDIMENT_TEST_FILE,
            error->message);
    } else {
        sed_sediment_set_env(sed);
    }


    {
        Suite* s = sed_compact_suite();
        SRunner* sr = srunner_create(s);

        srunner_run_all(sr, CK_NORMAL);
        n = srunner_ntests_failed(sr);
        srunner_free(sr);
    }

    sed_sediment_unset_env();

    return n;
}


