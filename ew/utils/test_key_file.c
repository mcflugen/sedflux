#define _GNU_SOURCE
#include <stdio.h>
#include <glib.h>
#include "utils/utils.h"

char* key_file_0[] = {
"[ 'first group' ]",
"key 0: 0",
"key 1: 1",
"[ 'second group' ]",
"key 0: 0",
"key 1: 1",
NULL
};

void
test_key_file_scan_stream (void)
{
  gchar* buffer = g_strjoinv ("\n", key_file_0);
  GError* error=NULL;
  Eh_key_file f;

  f = eh_key_file_scan_text (buffer, &error);
  g_assert (f);

  g_assert (eh_key_file_has_group (f,"first group"));
  g_assert (eh_key_file_has_group (f,"second group"));
  g_assert (!eh_key_file_has_group (f,"third group"));
  g_assert (!eh_key_file_has_group (f,"FIRST GROUP"));

  g_assert (eh_key_file_has_key (f, "first group", "key 0"));
  g_assert (eh_key_file_has_key (f, "first group", "key 1"));
  g_assert (eh_key_file_has_key (f, "first group", "KEY 1"));

  g_assert (eh_key_file_has_key (f, "second group", "key 0"));
  g_assert (eh_key_file_has_key (f, "second group", "key 1"));

  g_assert_cmpstr (eh_key_file_get_value (f, "first group", "key 0"), ==, "0");
  g_assert_cmpstr (eh_key_file_get_value (f, "first group", "key 1"), ==, "1");

  g_assert_cmpint (
    eh_key_file_get_int_value (f, "first group", "key 0"), ==, 0);
  g_assert_cmpint (
    eh_key_file_get_int_value (f, "first group", "key 1"), ==, 1);
}

int
main (int argc, char* argv[])
{
  eh_init_glib ();

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/utils/key_file/scan", &test_key_file_scan_stream);

  g_test_run ();
}

