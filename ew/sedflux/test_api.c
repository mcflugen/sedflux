#include <glib.h>

int
main (int argc, char* argv[])
{
  eh_init_glib ();

  if (!sed_test_setup_sediment ("sediment"))
    eh_exit (EXIT_FAILURE);

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/sedflux/api/initialize",&api_initialize);
  g_test_add_func ("/sedflux/api/run",&api_run);
  g_test_add_func ("/sedflux/api/finalize",&api_finalize);

  g_test_run ();
}

