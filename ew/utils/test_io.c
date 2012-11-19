#include <stdio.h>
#include <glib.h>
#include "utils/utils.h"
#include <eh_utils.h>

void
test_getline_one_line (void)
{
   gchar* name = NULL;
   FILE*  fp = eh_open_temp_file( NULL , &name );
   gchar* line_str = NULL;
   gsize  len      = 0;
   gchar* line_1 = "one line\n";
   gint   n;

   fprintf (fp, "%s", line_1);
   fclose (fp);

   fp = fopen (name, "r");
   n = getline (&line_str, &len, fp);
   fclose (fp);

   g_assert (line_str!=NULL);
   g_assert (n==strlen(line_1));
   g_assert (strlen(line_str)==9);
}

void
test_getline_no_new_line (void)
{
   gchar* name = NULL;
   FILE*  fp = eh_open_temp_file (NULL, &name);
   gchar* line_str = NULL;
   gsize  len      = 0;
   gchar* line_1 = "no new line char";
   gint   n;

   fprintf (fp, "%s", line_1);
   fclose (fp);

   fp = fopen (name, "r");
   n = getline (&line_str, &len, fp);
   fclose (fp);

   g_assert_cmpint (n, !=, -1);

   g_assert (line_str!=NULL);
   g_assert_cmpint (strlen (line_str), ==, strlen(line_1));
   g_assert_cmpstr (line_str, ==, line_1);

}

void
test_getline_empty_file (void)
{
   gchar* name = NULL;
   FILE*  fp = eh_open_temp_file (NULL, &name);
   gchar* line_str = NULL;
   gsize  len      = 0;
   gint   n;

   fprintf (fp, "");
   fclose (fp);

   fp = fopen(name, "r");
   n = getline (&line_str, &len, fp);
   fclose (fp);

   g_assert_cmpint (n, ==, -1);
   /*
   g_assert (line_str == NULL);
   g_assert_cmpint (len, ==, 0);
   g_assert_cmpint (strlen (line_str), ==, 0);
   */
}

void
test_getline_empty_line (void)
{
   gchar* name = NULL;
   FILE*  fp = eh_open_temp_file (NULL, &name);
   gchar* line_str = NULL;
   gsize  len      = 0;
   gsize  n;

   fprintf (fp, "\n");
   fclose (fp);

   fp = fopen (name, "r");
   n = getline (&line_str, &len, fp);
   fclose (fp);

   g_assert ( line_str!=NULL );
   g_assert_cmpint (n, ==, 1);
   g_assert_cmpint (strlen(line_str), ==, 1);
   g_assert (line_str[0] == '\n' );
}

void
test_getline_multi_line (void)
{
   gchar* name = NULL;
   FILE*  fp = eh_open_temp_file (NULL, &name);
   gchar* line_str = NULL;
   gsize  len      = 0;
   gchar* line_1   = "The first line\n";
   gchar* line_2   = "The second line without a new line char";
   gint   n;

   fprintf (fp, "%s", line_1);
   fprintf (fp, "%s", line_2);
   fclose (fp);

   fp = fopen (name, "r");
   getline (&line_str, &len, fp);
   n = getline (&line_str, &len, fp);
   fclose (fp);

   g_assert (line_str!=NULL );
   g_assert_cmpint (n, ==, strlen (line_2));
   g_assert_cmpint (strlen(line_str), ==, strlen(line_2));
   g_assert_cmpstr (line_str, ==, line_2);
}

void
test_getline_alloc (void) {
   gchar* name = NULL;
   FILE*  fp = eh_open_temp_file( NULL , &name );
   gchar* line_str = NULL;
   gsize  len      = 0;
   gchar* line_1 = "one line\n";
   gint   n;
   gchar* saved_line_ptr;

   len = 256;
   line_str = eh_new (gchar, len);
   saved_line_ptr = line_str;

   fprintf (fp, "%s", line_1);
   fclose (fp);

   fp = fopen (name, "r");
   n = getline (&line_str, &len, fp);
   fclose (fp);

   g_assert (line_str == saved_line_ptr);
   g_assert_cmpint (len, ==, 256);
   g_assert_cmpint (strlen(line_str), ==, n);
}

void
test_getline_realloc (void) {
   gchar* name = NULL;
   FILE* fp = eh_open_temp_file( NULL , &name );
   gchar* line_str = NULL;
   gsize len      = 0;
   gchar* line_1 = "one line\n";
   gint n;
   gchar* saved_line_ptr;
   int saved_len;

   len = 7;
   line_str = eh_new (gchar, len);
   saved_line_ptr = line_str;
   saved_len = len;

   fprintf (fp, "%s", line_1);
   fclose (fp);

   fp = fopen (name, "r");
   n = getline (&line_str, &len, fp);
   fclose (fp);

   g_assert_cmpint (len, >, saved_len);
   g_assert (strlen(line_str) == n);
}

int
main (int argc, char* argv[])
{
  eh_init_glib ();

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/utils/io/getline_1", &test_getline_one_line);
  g_test_add_func ("/utils/io/getline_no_eol", &test_getline_no_new_line);
  g_test_add_func ("/utils/io/getline_empty_file", &test_getline_empty_file);
  g_test_add_func ("/utils/io/getline_empty_line", &test_getline_empty_line);
  g_test_add_func ("/utils/io/getline_multi_line", &test_getline_multi_line);
  g_test_add_func ("/utils/io/getline_alloc", &test_getline_alloc);
  g_test_add_func ("/utils/io/getline_realloc", &test_getline_realloc);

  g_test_run ();
}

