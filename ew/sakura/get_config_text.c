#include <glib.h>


static const gchar* _DEFAULT_CONFIG[] = {
"sua",
"sub",
"entrainment constant, ea",
"entrainment constant, eb",
"drag coefficient",
"internal friction angle",
"width of channel",
"length of channel",
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
