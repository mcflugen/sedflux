#include <glib.h>


static const gchar* _DEFAULT_CONFIG[] = {
"diffusion constant",
"diffusion 1% depth",
"long-shore diffusion constant",
"cross-shore diffusion constant",
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
