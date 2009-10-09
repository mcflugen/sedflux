#include <stdlib.h>
#include <stdio.h>
#include <glib.h>

#include <utils/utils.h>
#include "sed_sediment.h"

gboolean
sed_test_setup_sediment (gchar* test_name)
{
  Sed_sediment s = NULL;
  GError* error = NULL;
  gchar* buffer = sed_sediment_default_text ();

  s = sed_sediment_scan_text (buffer, &error);
  g_free (buffer);

  eh_print_on_error (error, "%s", test_name);
  if (s)
    sed_sediment_set_env (s);
  else
    return FALSE;

  return TRUE;
}

