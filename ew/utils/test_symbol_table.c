#include <glib.h>
#include <eh_utils.h>

void
test_create_symbol_table (void)
{
   Eh_symbol_table t;

   t = eh_symbol_table_new ();

   g_assert (t!=NULL);
   g_assert_cmpint (eh_symbol_table_size (t), ==, 0);

   t = eh_symbol_table_destroy (t);

   g_assert (t==NULL);
}

void
test_add_value (void)
{
   Eh_symbol_table t;
   gchar* label = g_strdup("test label");
   gchar* value = g_strdup("test value");
   gchar* stored_value;

   t = eh_symbol_table_new ();

   eh_symbol_table_insert (t, label, value);

   g_assert_cmpint (eh_symbol_table_size (t), ==, 1);
   g_assert (eh_symbol_table_has_label (t, label));

   stored_value = eh_symbol_table_value (t, label);

   g_assert (stored_value!=value);
   g_assert (g_ascii_strcasecmp (stored_value, value)==0);

   eh_free( stored_value );

   stored_value = eh_symbol_table_lookup (t, label);
   g_assert (stored_value!=value);

   t = eh_symbol_table_destroy (t);
}

int
main (int argc, char* argv[])
{
  eh_init_glib ();

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/utils/sym_table/create", &test_create_symbol_table);
  g_test_add_func ("/utils/sym_table/add_value", &test_add_value);

  g_test_run ();
}

