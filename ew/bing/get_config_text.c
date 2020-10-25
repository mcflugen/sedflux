#include <glib.h>


const gchar* _DEFAULT_BING_CONFIG[] = {
    "yield strength (Pa): 100",
    "kinematic viscosity: 0.083",
    "artificial viscosity: 0.5",
    "time step (s): 0.02",
    "maximum run time (min): 240",
    NULL
};


const gchar*
get_config_text(const gchar* file)
{
    if (g_ascii_strcasecmp(file, "config") == 0) {
        return g_strjoinv("\n", (gchar**)_DEFAULT_BING_CONFIG);
    } else {
        return NULL;
    }
}
