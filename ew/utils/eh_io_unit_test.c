#include <eh_utils.h>
#include <check.h>

START_TEST(test_getline_one_line)
{
    gchar* name = NULL;
    FILE*  fp = eh_open_temp_file(NULL, &name);
    gchar* line_str = NULL;
    gsize  len      = 0;
    gchar* line_1 = "one line\n";
    gint   n;

    fprintf(fp, "%s", line_1);
    fclose(fp);

    fp = fopen(name, "r");
    n = getline(&line_str, &len, fp);
    fclose(fp);

    fail_unless(line_str != NULL);
    fail_unless(n == strlen(line_1));
    fail_unless(strlen(line_str) == 9);

    fclose(fp);
}
END_TEST

START_TEST(test_getline_no_new_line)
{
    gchar* name = NULL;
    FILE*  fp = eh_open_temp_file(NULL, &name);
    gchar* line_str = NULL;
    gsize  len      = 0;
    gchar* line_1 = "no new line char";
    gint   n;

    fprintf(fp, "%s", line_1);
    fclose(fp);

    fp = fopen(name, "r");
    n = getline(&line_str, &len, fp);
    fclose(fp);

    fail_unless(line_str != NULL);
    fail_unless(n != -1);
    fail_unless(strlen(line_str) == strlen(line_1));
    fail_unless(strcmp(line_str, line_1) == 0);
}
END_TEST

START_TEST(test_getline_empty_file)
{
    gchar* name = NULL;
    FILE*  fp = eh_open_temp_file(NULL, &name);
    gchar* line_str = NULL;
    gsize  len      = 0;
    gint   n;

    fprintf(fp, "");
    fclose(fp);

    fp = fopen(name, "r");
    n = getline(&line_str, &len, fp);
    fclose(fp);

    fail_unless(line_str != NULL);
    fail_unless(n == -1);
    fail_unless(strlen(line_str) == 0);
}
END_TEST

START_TEST(test_getline_empty_line)
{
    gchar* name = NULL;
    FILE*  fp = eh_open_temp_file(NULL, &name);
    gchar* line_str = NULL;
    gsize  len      = 0;
    gsize  n;

    fprintf(fp, "\n");
    fclose(fp);

    fp = fopen(name, "r");
    n = getline(&line_str, &len, fp);
    fclose(fp);

    fail_unless(line_str != NULL);
    fail_unless(n == 1);
    fail_unless(strlen(line_str) == 1);
    fail_unless(line_str[0] == '\n');
}
END_TEST

START_TEST(test_getline_multi_line)
{
    gchar* name = NULL;
    FILE*  fp = eh_open_temp_file(NULL, &name);
    gchar* line_str = NULL;
    gsize  len      = 0;
    gchar* line_1   = "The first line\n";
    gchar* line_2   = "The second line without a new line char";
    gint   n;

    fprintf(fp, "%s", line_1);
    fprintf(fp, "%s", line_2);
    fclose(fp);

    fp = fopen(name, "r");
    getline(&line_str, &len, fp);
    n = getline(&line_str, &len, fp);
    fclose(fp);

    fail_unless(line_str != NULL);
    fail_unless(n == -1);
    fail_unless(strlen(line_str) == strlen(line_2));
    fail_unless(strcmp(line_str, line_2) == 0);
}
END_TEST

Suite*
eh_io_suite(void)
{
    Suite* s = suite_create("Input/output functions");
    TCase* test_case_core = tcase_create("Core");

    suite_add_tcase(s, test_case_core);

    tcase_add_test(test_case_core, test_getline_one_line);
    tcase_add_test(test_case_core, test_getline_no_new_line);
    tcase_add_test(test_case_core, test_getline_empty_file);
    tcase_add_test(test_case_core, test_getline_empty_line);
    tcase_add_test(test_case_core, test_getline_multi_line);

    return s;
}

int
test_io(void)
{
    int n;

    {
        Suite* s = eh_io_suite();
        SRunner* sr = srunner_create(s);

        srunner_run_all(sr, CK_NORMAL);
        n = srunner_ntests_failed(sr);
        srunner_free(sr);
    }

    return n;
}

