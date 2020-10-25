#include <glib.h>


static const gchar* _DEFAULT_CONFIG[] = {
    "effective elastic thickness",
    "Youngs modulus",
    "relaxation time",
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
