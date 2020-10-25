//---
//
// This file is part of sedflux.
//
// sedflux is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// sedflux is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with sedflux; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---

#include <stdio.h>

#include <math.h>
#include <string.h>
#include <unistd.h>

#include <utils/utils.h>
#include <sed/sed_sedflux.h>

#include "compact.h"

/*** Self Documentation ***/
char* help_msg[] = {
    "                                                                             ",
    " compact [options] [parameters]  [filein]                                    ",
    "  compact a sedflux profile.                                                 ",
    "                                                                             ",
    " Options                                                                     ",
    "  -v          - be verbose. [off]                                            ",
    "  -h          - print this help message.                                     ",
    "  -hfile      - print a help message on the file formats.                    ",
    "  -a          - input will be ascii.  This is the default.                   ",
    "  -b          - input will be a binary file from a sedflux dump.             ",
    "                                                                             ",
    " Parameters                                                                  ",
    "  -pdy=value  - set the height of sediment cells in the output ASCII file to ",
    "                be value.  The deafult is to not rebin the cells.            ",
    "                                                                             ",
    " Files                                                                       ",
    "  -fsed=file  - specify the name of the file containing the sediment         ",
    "                information. [compact.sed]                                   ",
    "  -fin=file   - specify the name of the input file. [stdin]                  ",
    "  -fout=file  - specify the name of the output file. [stdout]                ",
    "                                                                             ",
    NULL
};

char* file_msg[] = {
    "                                                                             ",
    " The input file consists of a header line followed by data lines.  The header",
    " line consists of a single number stating the number of data lines to follow.",
    " For each cell in the sediment column that is to be compacted, the input file",
    " gives the thickness of a cell and the fractions of each grain type that make",
    " up that cell.  Each line of the file describes packages of sediment with the",
    " first being the top of the sediment column.  The lines are divided into at  ",
    " least two columns.  The first is the thickness (in meters) of that sediment ",
    " package, and the rest are the fractions of each grain that compose that     ",
    " cell.  The fractions are renormalized and so don't have to add up to one.   ",
    "                                                                             ",
    NULL
};

Sed_column
_scan_sediment_column(const gchar* file, double dy, GError** error);
gint
_print_sediment_column(const gchar* file, Sed_column s, GError** error);

static double   dy       = 1.0;
static gboolean rebin    = FALSE;
static gboolean verbose  = FALSE;
static gboolean version  = FALSE;
static gchar*   infile   = NULL;
static gchar*   outfile  = NULL;
static gchar*   sedfile  = NULL;
static gboolean diag     = FALSE;

static GOptionEntry entries[] = {
    { "cell-height", 'h', 0, G_OPTION_ARG_DOUBLE, &dy, "Cell thickness (m)", "dz"   },
    { "rebin", 'r', 0, G_OPTION_ARG_NONE, &rebin, "Rebin cells after compaction", NULL   },
    { "verbose", 'V', 0, G_OPTION_ARG_NONE, &verbose, "Be verbose", NULL   },
    { "version", 'v', 0, G_OPTION_ARG_NONE, &version, "Print version number", NULL   },
    { "in-file", 'i', 0, G_OPTION_ARG_FILENAME, &infile, "Input file", "FILE" },
    { "out-file", 'o', 0, G_OPTION_ARG_FILENAME, &outfile, "Output file", "FILE" },
    { "sed-file", 's', 0, G_OPTION_ARG_FILENAME, &sedfile, "Sediment file", "FILE" },
    { "diag",  0, 0, G_OPTION_ARG_NONE, &diag, "Run a diagnostics", NULL   },
    { NULL }
};

gint
main(gint argc, gchar* argv[])
{
    GError* error = NULL;

    eh_init_glib();

    { /* Parse command line options, set values */
        GOptionContext* context = g_option_context_new("Run compaction model.");

        g_option_context_add_main_entries(context, entries, NULL);

        if (!g_option_context_parse(context, &argc, &argv, &error)) {
            eh_error("Error parsing command line arguments: %s", error->message);
        }
    }

    if (!error && version) {
        /* Print version number and exit */
        eh_fprint_version_info(stdout, COMPACTION_PROGRAM_NAME,
            COMPACTION_MAJOR_VERSION,
            COMPACTION_MINOR_VERSION,
            COMPACTION_MICRO_VERSION);
        eh_exit(EXIT_SUCCESS);
    }

    if (!error) {
        /* Run the model */
        Sed_column   col;
        Sed_sediment sed;

        sed = sed_sediment_scan(sedfile, &error);

        if (!error) {
            /* Setup the sediment environment */
            sed_sediment_set_env(sed);
            sed_sediment_destroy(sed);
        }

        if (!error) {
            col = _scan_sediment_column(infile, dy, &error);
        }

        if (!error) {
            /* Compact the column of sediment */
            Sed_diag d = NULL;

            if (diag) {
                d = sed_diag_new_target_column(col);
                sed_diag_start(d);
            }

            eh_debug("Compact the column.");

            compact(col);

            if (rebin) {
                sed_column_rebin(col);
            }

            if (diag) {
                sed_diag_stop(d);
                sed_diag_fprint(stderr, d);
                sed_diag_destroy(d);
            }

            eh_debug("Write the output bulk densities.");
            _print_sediment_column(outfile, col, &error);
        }

        eh_exit_on_error(error, "Error in compaction program");

        sed_column_destroy(col);
        sed_sediment_unset_env();
    }

    return EXIT_SUCCESS;
}

Sed_column
_scan_sediment_column(const gchar* file, double dy, GError** error)
{
    Sed_column s = NULL;

    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    eh_require(dy > 0);

    if (dy > 0) {
        GError*  tmp_err = NULL;
        double** data    = NULL;
        gint     n_cells;
        gint     n_grains;

        data = eh_dlm_read(file, ";", &n_cells, &n_grains, &tmp_err);

        n_grains -= 1;

        if (!tmp_err && n_grains != sed_sediment_env_n_types()) {
            g_set_error(&tmp_err, COMPACT_ERROR, COMPACT_ERROR_INPUT_FILE,
                "Number of grain types in sediment file and input file are unequal (%d!=%d)",
                sed_sediment_env_n_types(), n_grains);
        } else {
            Sed_cell c = sed_cell_new_sized(n_grains, dy, data[0] + 1);
            gssize   i;

            s = sed_column_new(n_cells);
            sed_column_set_z_res(s, dy);

            eh_debug("Fill the sediment column with sediment");

            for (i = 0 ; i < n_cells ; i++) {
                sed_cell_set_fraction(c, data[n_cells - 1 - i] + 1);
                sed_column_add_cell(s, c);
            }

            for (i = 0 ; i < n_cells ; i++) {
                sed_column_resize_cell(s, i, data[n_cells - 1 - i][0]);
            }

            sed_cell_destroy(c);
        }

        if (tmp_err) {
            g_propagate_error(error, tmp_err);
            s = sed_column_destroy(s);
        }

        eh_free_2(data);
    }

    return s;
}

gint
_print_sediment_column(const gchar* file, Sed_column s, GError** error)
{
    gint n = 0;

    eh_return_val_if_fail(error == NULL || *error == NULL, 0);

    eh_require(s);

    if (s) {
        GError*  tmp_err  = NULL;
        gint     n_rows   = sed_column_len(s);
        gint     n_grains = sed_sediment_env_n_types();
        double** data     = eh_new_2(double, n_rows, n_grains + 1);
        Sed_cell c        = NULL;
        gint     i, j;

        for (i = 0 ; i < n_rows ; i++) {
            c = sed_column_nth_cell(s, n_rows - 1 - i);
            data[i][0] = sed_cell_size(c);

            for (j = 0 ; j < n_grains ; j++) {
                data[i][j + 1] = sed_cell_nth_fraction(c, j);
            }
        }

        n = eh_dlm_print(file, ";", (const double**)data, n_rows, n_grains + 1, &tmp_err);

        if (tmp_err) {
            g_propagate_error(error, tmp_err);
        }

        eh_free_2(data);
    }

    return n;
}

