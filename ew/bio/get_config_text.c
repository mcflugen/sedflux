#include <glib.h>


const static gchar* _DEFAULT_CONFIG[] = {
    "depth of bioturbation",
    "bioturbation diffusion coefficient",
    NULL
};


gchar*
get_config_text(const gchar* file)
{
    if (g_ascii_strcasecmp(file, "config") == 0) {
        return g_strjoinv("\n", (gchar**)_DEFAULT_CONFIG);
    } else {
        return NULL;
    }
}
