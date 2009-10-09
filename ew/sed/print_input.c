#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "sed_input_files.h"

#define SED_PRINT_PROGRAM_NAME "sed-print-input"
#define SED_PRINT_MAJOR_VERSION (0)
#define SED_PRINT_MINOR_VERSION (1)
#define SED_PRINT_MICRO_VERSION (0)

static gchar*   file       = "sediment";
static gint     verbose    = 0;
static gboolean version    = FALSE;

static GOptionEntry entries[] = {
  { "file", 'f', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING, &file,
    "Input file to print", "FILE" },
  { "verbose", 'V', 0, G_OPTION_ARG_INT, &verbose,
    "Verbosity level", "N"} ,
  { "version", 'v', 0, G_OPTION_ARG_NONE, &version,
    "Version number", NULL} ,
  { NULL }
};

int
main (int argc, char* argv[])
{
  GError* error = NULL;
  GOptionContext* context = g_option_context_new (
                              "Print example sedflux input files.");

  g_option_context_add_main_entries (context, entries, NULL);

  g_option_context_parse (context, &argc, &argv, &error);
  eh_print_on_error (error, "%s", SED_PRINT_PROGRAM_NAME);
  eh_exit_on_error (error);

  if (version)
  {
    eh_fprint_version_info (stdout, SED_PRINT_PROGRAM_NAME,
                                    SED_PRINT_MAJOR_VERSION,
                                    SED_PRINT_MINOR_VERSION,
                                    SED_PRINT_MICRO_VERSION);
    eh_exit (EXIT_SUCCESS);
  }

  {
    char** line = NULL;

    if (g_ascii_strcasecmp (file, "SEDIMENT")==0)
      line = _default_sediment_file;
    else if (g_ascii_strcasecmp (file, "RIVER")==0)
      line = _default_hydro_inline_file;
    else
      g_assert_not_reached ();

    for (; *line; line++)
      fprintf (stdout, "%s\n", *line);
  }

  return EXIT_SUCCESS;
}

