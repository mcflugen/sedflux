#include <glib.h>


static const gchar* _DEFAULT_CONFIG[] = {
"background ocean concentration",
"velocity of coastal current",
"maximum plume width",
"number of grid nodes in cross-shore",
"number of grid nodes in river mouth",
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
