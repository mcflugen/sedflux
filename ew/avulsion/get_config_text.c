#include <glib.h>


const gchar* _DEFAULT_AVULSION_CONFIG [] = {
"standard deviation: 0.75",
"minimum angle (degrees): 0",
"maximum angle (degrees): 180",
"river name: Blackfoot",
"hinge point: 50, 0",
"fraction of sediment remaining in plane: 1",
"river can branch? (yes or no): no",
"seed for random number generator: 1945",
NULL
};


gchar*
get_config_text (const gchar* file)
{
  if (g_ascii_strcasecmp (file, "config")==0)
    return g_strjoinv ("\n", (gchar**)_DEFAULT_AVULSION_CONFIG);
  else
    return NULL;
}
