#include "utils/utils.h"
#include <check.h>

#include "sed_wave.h"

START_TEST(test_sed_wave_new)
{
    {
        Sed_wave w = sed_wave_new(0, 2, 3);

        fail_if(w == NULL, "NULL returned instead of a new wave");
        fail_unless(fabs(sed_wave_height(w)) < 1e-12, "Newly created has wrong height");
        fail_unless(fabs(sed_wave_number(w)    - 2) < 1e-12, "Newly created has wrong number");
        fail_unless(fabs(sed_wave_frequency(w) - 3) < 1e-12,
            "Newly created has wrong frequency");

        sed_wave_destroy(w);
    }

    {
        Sed_wave w = sed_wave_new(0, 0, -1);
        fail_unless(w == NULL, "Wave can not have negative frequency");
    }

    {
        Sed_wave w = sed_wave_new(0, -1, 0);
        fail_unless(w == NULL, "Wave can not have negative number");
    }

    {
        Sed_wave w = sed_wave_new(-1, 0, 0);
        fail_unless(w == NULL, "Wave can not have negative height");
    }

}
END_TEST

START_TEST(test_sed_wave_copy)
{
    {
        Sed_wave dest = sed_wave_new(0, 0, 0);
        Sed_wave src  = sed_wave_new(1, 2, 3);
        Sed_wave temp = dest;

        dest = sed_wave_copy(dest, src);

        fail_unless(sed_wave_is_same(dest, src), "Wave not copied correctly");
        fail_if(dest == src, "Wave not copied");
        fail_unless(dest == temp, "Wave not copied to correct location");

        sed_wave_destroy(src);
        sed_wave_destroy(dest);
    }
}
END_TEST

Suite*
sed_wave_suite(void)
{
    Suite* s = suite_create("Sed_wave");
    TCase* test_case_core = tcase_create("Core");

    suite_add_tcase(s, test_case_core);

    tcase_add_test(test_case_core, test_sed_wave_new);
    tcase_add_test(test_case_core, test_sed_wave_copy);

    return s;
}

int
test_sed_wave(void)
{
    int n;

    {
        Suite* s = sed_wave_suite();
        SRunner* sr = srunner_create(s);

        srunner_run_all(sr, CK_NORMAL);
        n = srunner_ntests_failed(sr);
        srunner_free(sr);
    }

    return n;
}


