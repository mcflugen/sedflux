#include <glib.h>


static const gchar* _DEFAULT_CONFIG[] = {
"time step",
"duration of squall",
NULL
};

gchar*
get_config_text (const gchar* file)
{
  if (g_ascii_strcasecmp (file, "config")==0)
    return g_strjoinv ("\n", (gchar**)_DEFAULT_CONFIG);
  else
    return NULL;
}
