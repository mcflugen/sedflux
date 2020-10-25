#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gmodule.h>

#include "sed_input_files.h"
#include "datadir_path.h"

gchar*
get_input_file_contents(const gchar* name, const gchar* file,
    GError** error);

typedef enum {
    SED_PRINT_ERROR_MODULE_NOT_FOUND,
    SED_PRINT_ERROR_SYMBOL_NOT_FOUND,
    SED_PRINT_ERROR_SYMBOL_IS_NULL,
    SED_PRINT_ERROR_INPUT_NOT_FOUND,
}
Sed_print_error;

#define SED_PRINT_ERROR     sed_print_error_quark()
GQuark
sed_print_error_quark(void)
{
    return g_quark_from_static_string("sed-print-error-quark");
}

#define SED_PRINT_PROGRAM_NAME "sed-print-input"
#define SED_PRINT_MAJOR_VERSION (0)
#define SED_PRINT_MINOR_VERSION (1)
#define SED_PRINT_MICRO_VERSION (0)

static gchar*   file       = NULL;
static gchar*   module     = NULL;
static gint     verbose    = 0;
static gboolean version    = FALSE;

static GOptionEntry entries[] = {
    {
        "file", 'f', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING, &file,
        "Input file to print", "FILE"
    },
    {
        "module", 'm', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING, &module,
        "Input file module", "MODULE"
    },
    {
        "verbose", 'V', 0, G_OPTION_ARG_INT, &verbose,
        "Verbosity level", "N"
    },
    {
        "version", 'v', 0, G_OPTION_ARG_NONE, &version,
        "Version number", NULL
    },
    { NULL }
};

int
main(int argc, char* argv[])
{
    GError* error = NULL;
    GOptionContext* context = g_option_context_new(
            "Print example sedflux input files.");

    g_option_context_add_main_entries(context, entries, NULL);

    g_option_context_parse(context, &argc, &argv, &error);
    eh_print_on_error(error, "%s", SED_PRINT_PROGRAM_NAME);
    eh_exit_on_error(error);

    if (version) {
        /* Print version number and exit. */
        eh_fprint_version_info(stdout, SED_PRINT_PROGRAM_NAME,
            SED_PRINT_MAJOR_VERSION,
            SED_PRINT_MINOR_VERSION,
            SED_PRINT_MICRO_VERSION);
        eh_exit(EXIT_SUCCESS);
    }

    { /* Print the input file. */
        char** line = NULL;

        /*
            if (g_ascii_strcasecmp (file, "SEDIMENT")==0)
              line = _default_sediment_file;
            else if (g_ascii_strcasecmp (file, "RIVER")==0)
              line = _default_hydro_inline_file;
            else if (g_ascii_strcasecmp (file, "INIT")==0)
              line = _default_init_file;
            else if (g_ascii_strcasecmp (file, "BATHY-1D")==0)
              line = _default_1d_bathy_file;
            else if (g_ascii_strcasecmp (file, "BATHY-2D")==0)
              line = _default_2d_bathy_file;
            else
              g_assert_not_reached ();

            for (; *line; line++)
              fprintf (stdout, "%s\n", *line);
        */
        if (module) {
            gchar* buffer = NULL;
            GError* error = NULL;

            buffer = get_input_file_contents(module, file, &error);

            if (!error) {
                fprintf(stdout, "%s\n", buffer);
            } else {
                eh_print_on_error(error, "%s", SED_PRINT_PROGRAM_NAME);
            }
        }
    }

    return EXIT_SUCCESS;
}

typedef gchar* (*FileContentsFunc)(const gchar*);

gchar*
get_input_file_contents(const gchar* mod_name, const gchar* file,
    GError** error)
{
    gchar* text = NULL;
    FileContentsFunc get_contents;
    GModule* module;
    gchar* name = NULL;

    name = g_module_build_path(PLUGINDIR, mod_name);

    module = g_module_open(name, G_MODULE_BIND_LAZY);

    if (!module) {
        g_set_error(error, SED_PRINT_ERROR, SED_PRINT_ERROR_MODULE_NOT_FOUND,
            "%s", g_module_error());
        return NULL;
    }

    if (!g_module_symbol(module, "get_config_text",
            (gpointer*)&get_contents)) {
        g_set_error(error, SED_PRINT_ERROR, SED_PRINT_ERROR_SYMBOL_NOT_FOUND,
            "%s: %s", mod_name, g_module_error());

        if (!g_module_close(module)) {
            g_warning("%s: %s", mod_name, g_module_error());
        }

        return NULL;
    }

    if (get_contents == NULL) {
        g_set_error(error, SED_PRINT_ERROR, SED_PRINT_ERROR_SYMBOL_IS_NULL,
            "symbol %s is NULL", "get_config_text");

        if (!g_module_close(module)) {
            g_warning("%s: %s", mod_name, g_module_error());
        }

        return NULL;
    }

    text = get_contents(file);

    if (!text)
        g_set_error(error, SED_PRINT_ERROR, SED_PRINT_ERROR_INPUT_NOT_FOUND,
            "module %s does not have input file %s", mod_name, file);

    if (!g_module_close(module)) {
        g_warning("%s: %s", mod_name, g_module_error());
    }

    return text;
}

gchar*
get_config_text(const gchar* file)
{
    if (g_ascii_strcasecmp(file, "config") == 0) {
        return g_strjoinv("\n", (gchar**)_default_init_file);
    }

    if (g_ascii_strcasecmp(file, "bathy-1d") == 0) {
        return g_strjoinv("\n", (gchar**)_default_1d_bathy_file);
    }

    if (g_ascii_strcasecmp(file, "bathy-2d") == 0) {
        return g_strjoinv("\n", (gchar**)_default_2d_bathy_file);
    }

    if (g_ascii_strcasecmp(file, "hydro") == 0) {
        return g_strjoinv("\n", (gchar**)_default_hydro_inline_file);
    }

    if (g_ascii_strcasecmp(file, "sediment") == 0) {
        return g_strjoinv("\n", (gchar**)_default_sediment_file);
    }

    if (g_ascii_strcasecmp(file, "process") == 0) {
        return g_strjoinv("\n", (gchar**)_default_sediment_file);
    } else {
        return NULL;
    }
}
