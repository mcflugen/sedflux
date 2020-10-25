#include <glib.h>


static gchar* _DEFAULT_CONFIG[] = {
    NULL
};


gchar*
get_config_text(const gchar* file)
{
    if (g_ascii_strcasecmp(file, "config") == 0) {
        return g_strjoinv("\n", _DEFAULT_CONFIG);
    } else {
        return NULL;
    }
}
