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
#include <utils/utils.h>
#include <sed/sed_sedflux.h>

static gchar**  in_file    = NULL;
static gchar*   out_file   = NULL;
static gint     in_type    = 0;
static gint     out_type   = 0;
static double   dt         = 365.;
static double   fraction   = 1.;
static gint     n_recs     = -1;
static gint     buf_len    = 365;
static gint     events     = 5;
static gint     start      = 0;
static gboolean just_events = FALSE;
static gboolean version    = FALSE;
static gint     verbosity  = 5;
static gboolean debug      = FALSE;
static gboolean info       = FALSE;
static gint     to         = G_BYTE_ORDER;
static gint     from       = G_BYTE_ORDER;

static gboolean
parse_file_list(const gchar* name, const gchar* value, gpointer data, GError** error);
static gboolean
parse_file_type(const gchar* name, const gchar* value, gpointer data, GError** error);
static gboolean
parse_byte_order(const gchar* name, const gchar* value, gpointer data, GError** error);

static void
write_data(FILE* fp, Sed_hydro* a, gint type, gint order);

GOptionEntry entries[] = {
    { "in-file", 'i', 0, G_OPTION_ARG_CALLBACK, parse_file_list, "Input file", "<file>" },
    { "in-type",  0, 0, G_OPTION_ARG_CALLBACK, &parse_file_type, "Input file type", "TYPE" },
    { "out-type",  0, 0, G_OPTION_ARG_CALLBACK, &parse_file_type, "Output file type", "TYPE" },
    { "dt", 'T', 0, G_OPTION_ARG_DOUBLE, &dt, "Duration (days)", "TIME" },
    { "fraction", 'f', 0, G_OPTION_ARG_DOUBLE, &fraction, "Fraction of sediment", "FRAC" },
    { "n-recs", 'N', 0, G_OPTION_ARG_INT, &n_recs, "Number of records", "N" },
    { "buffer", 'l', 0, G_OPTION_ARG_INT, &buf_len, "Buffer length", "LEN" },
    { "events", 'n', 0, G_OPTION_ARG_INT, &events, "Number of events", "N" },
    { "start-rec", 's', 0, G_OPTION_ARG_INT, &start, "Start record", "N" },
    { "just-events", 'e', 0, G_OPTION_ARG_NONE, &just_events, "Don't include non-floods", NULL },
    { "verbose", 'V', 0, G_OPTION_ARG_INT, &verbosity, "Verbosity level", "n" },
    { "version", 'v', 0, G_OPTION_ARG_NONE, &version, "Version number", NULL },
    { "debug", 'd', 0, G_OPTION_ARG_NONE, &debug, "Write debug messages", NULL },
    { "info",  0, 0, G_OPTION_ARG_NONE, &info, "Print file info", NULL },
    { "swap",  0, 0, G_OPTION_ARG_CALLBACK, parse_byte_order, "Swap byte order", NULL },
    { "to",  0, 0, G_OPTION_ARG_CALLBACK, parse_byte_order, "Destination byte order", "[big|little]" },
    { "from",  0, 0, G_OPTION_ARG_CALLBACK, parse_byte_order, "Source byte order", "[big|little]" },
    { "join",  0, 0, G_OPTION_ARG_CALLBACK, parse_file_list, "Files to join", "file1[,file2[,file3]]" },
    { NULL }
};

gboolean
parse_file_list(const gchar* name, const gchar* value, gpointer data, GError** error)
{
    gboolean success = FALSE;

    eh_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    if (name && value) {
        GError* tmp_err   = NULL;

        in_file  = g_strsplit(value, ",", 0);

        if (!in_file) {
            g_set_error(&tmp_err,
                G_OPTION_ERROR,
                G_OPTION_ERROR_FAILED,
                "Failed to parse comma-separated list of files to concatenate");
        }

        if (tmp_err) {
            g_propagate_error(error, tmp_err);
            success = FALSE;
        } else {
            success = TRUE;
        }
    }

    return success;
}

gboolean
parse_file_type(const gchar* name, const gchar* value, gpointer data, GError** error)
{
    gboolean success = FALSE;

    eh_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    if (name && value) {
        GError* tmp_err = NULL;
        gint    format  = -1;

        if (g_ascii_strcasecmp(value, "hydrotrend") == 0) {
            format  = 0;
        } else if (g_ascii_strcasecmp(value, "ascii") == 0) {
            format  = 1;
        } else if (g_ascii_strcasecmp(value, "hyperpycnal") == 0) {
            format  = 2;
        } else if (g_ascii_strcasecmp(value, "hypopycnal") == 0) {
            format  = 3;
        } else {
            g_set_error(&tmp_err,
                G_OPTION_ERROR,
                G_OPTION_ERROR_BAD_VALUE,
                "Unknown file format (%s): must be either HYDROTREND or ASCII", value);
        }

        if (g_ascii_strcasecmp(name, "--in-type") == 0) {
            in_type  = format;
        } else if (g_ascii_strcasecmp(name, "--out-type") == 0) {
            out_type = format;
        } else {
            g_set_error(&tmp_err,
                G_OPTION_ERROR,
                G_OPTION_ERROR_FAILED,
                "Invalid option name (%s)", name);
        }

        if (tmp_err) {
            g_propagate_error(error, tmp_err);
            success = FALSE;
        } else {
            success = TRUE;
        }
    }

    return success;
}

gboolean
parse_byte_order(const gchar* name, const gchar* value, gpointer data, GError** error)
{
    gboolean success = FALSE;

    eh_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    if (name && value) {
        GError* tmp_err = NULL;
        gint*   byte_order;

        if (g_ascii_strcasecmp(name, "--SWAP") != 0
            && g_ascii_strcasecmp(name, "--TO") != 0
            && g_ascii_strcasecmp(name, "--FROM") != 0) {
            success = FALSE;
            g_set_error(&tmp_err,
                G_OPTION_ERROR,
                G_OPTION_ERROR_FAILED,
                "Invalid option name (%s)", name);
        } else if (g_ascii_strcasecmp(name, "--SWAP") == 0) {
            from = G_BYTE_ORDER;

            if (G_BYTE_ORDER == G_BIG_ENDIAN) {
                to = G_LITTLE_ENDIAN;
            } else {
                to = G_BIG_ENDIAN;
            }
        } else {
            if (g_ascii_strcasecmp(name, "--TO") == 0) {
                byte_order = &to;
            } else if (g_ascii_strcasecmp(name, "--FROM") == 0) {
                byte_order = &from;
            }

            if (g_ascii_strcasecmp(value, "BIG") == 0
                || g_ascii_strcasecmp(value, "BIG-ENDIAN") == 0) {
                success = TRUE;
                *byte_order = G_BIG_ENDIAN;
            } else if (g_ascii_strcasecmp(value, "LITTLE") == 0
                || g_ascii_strcasecmp(value, "LITTLE-ENDIAN") == 0) {
                success = TRUE;
                *byte_order = G_LITTLE_ENDIAN;
            } else {
                success = FALSE;
                g_set_error(&tmp_err,
                    G_OPTION_ERROR,
                    G_OPTION_ERROR_BAD_VALUE,
                    "Unknown byte order (%s): must be either BIG-ENDIAN or LITTLE-ENDIAN", value);
            }
        }

        if (tmp_err) {
            g_propagate_error(error, tmp_err);
        }
    }

    return success;
}

#define byte_order_s( o ) ( (o==G_BIG_ENDIAN)?"big endian":"little endian" )

gint
main(gint argc, gchar* argv[])
{
    GError*         error   = NULL;
    FILE*           fp_out  = stdout;
    FILE**          fp_in   = NULL;

    { /* Parse command line options.  Exit if there is an error. */
        GOptionContext* context = g_option_context_new("Read a HydroTrend river file");

        //g_thread_init( NULL );
        eh_init_glib();
        g_log_set_handler(NULL, G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL, &eh_logger, NULL);

        g_option_context_add_main_entries(context, entries, NULL);

        if (!g_option_context_parse(context, &argc, &argv, &error)) {
            eh_error("Error parsing command line arguments: %s", error->message);
        }
    }

    if (version) {
        eh_fprint_version_info(stdout, "sedflux-read-hydro", 0, 1, 0), exit(0);
    }

    if (debug) {
        g_setenv("SEDFLUX_READ_HYDRO", "TRUE", TRUE);
    }

    eh_set_verbosity_level(verbosity);

    if (in_file) {
        /* Open all of the input files.  Exit if there is an error. */
        gchar** file;
        FILE*   fp;

        for (file = in_file ; *file ; file++) {
            fp = eh_fopen_error(*file, "r", &error);

            if (fp) {
                eh_strv_append((gchar***)&fp_in, (gchar*)fp);
            } else {
                eh_exit_on_error(error, "sedflux-read-hydro");
            }
        }
    } else {
        fp_in    = eh_new0(FILE*, 2);
        fp_in[0] = stdin;
    }

    if (info) {
        /* Print some info about the input file(s) and exit. */
        Sed_hydrotrend_header* h         = sed_hydrotrend_join_header_from_byte_order(fp_in,
                from, &error);
        gchar*                 file_list = g_strjoinv(";", in_file);

        eh_info("File name             : %s", file_list);
        eh_info("Number of grain sizes : %d", h->n_grains);
        eh_info("Number of seasons     : %d", h->n_seasons);
        eh_info("Number of records     : %d", h->n_samples);
        eh_info("Number of years       : %f", h->n_samples / (double)h->n_seasons);
        eh_info("Comment               : %s", h->comment);
        eh_info("Byte order            : %s", byte_order_s(from));

        eh_free(file_list);

        return EXIT_SUCCESS;
    }

    if (out_file) {
        /* Open the output file.  Exit if there is an error. */
        fp_out = eh_fopen_error(out_file, "w", &error);
        eh_exit_on_error(error, "sedflux-read-hydro");
    }

    eh_info("First record           : %d", start);
    eh_info("Number of records      : %d", n_recs);
    eh_info("Buffer length          : %d", buf_len);
    eh_info("Fraction of load       : %f", fraction);
    eh_info("Source byte order      : %s", byte_order_s(from));
    eh_info("Destination byte order : %s", byte_order_s(to));

    eh_watch_int(out_type);

    { /* Operate on the hydro file. */
        FILE**                 fp       = NULL;
        GError*                error    = NULL;
        Sed_hydrotrend_header* h        = sed_hydrotrend_join_header_from_byte_order(fp_in,
                from, &error);
        gint                   tot_recs = n_recs;
        eh_watch_ptr(h);
        eh_exit_on_error(error, "sedflux-read-hydro");

        if (n_recs <= 0) {
            tot_recs = h->n_samples - start;
        }

        if (out_type == 0) {
            sed_hydrotrend_write_header_to_byte_order(fp_out, h->n_grains, h->n_seasons, tot_recs,
                h->comment, to);
        }

        eh_watch_int(tot_recs);

        h = sed_hydrotrend_header_destroy(h);

        for (fp = fp_in ; *fp ; fp++) {
            /* If the header is valid, go to work. */
            gint       i;
            Sed_hydro* all_recs  = NULL;
            Sed_hydro* big_recs  = NULL;
            gint       top_rec   = 0;
            gint       top_block = 0;
            eh_watch_ptr(fp);
            tot_recs = n_recs;
            eh_watch_int(tot_recs);

            h = sed_hydrotrend_read_header_from_byte_order(*fp, from);
            eh_watch_ptr(h);
            eh_watch_int(h->n_grains);
            eh_watch_int(h->n_seasons);
            eh_watch_int(h->n_samples);
            eh_watch_str(h->comment);

            if (n_recs <= 0) {
                tot_recs = h->n_samples - start;
            }

            //         if ( out_type==0 ) sed_hydrotrend_write_header_to_byte_order( fp_out , h->n_grains , h->n_seasons , n_recs , h->comment , to );

            eh_watch_int(tot_recs);
            //top_rec   = start + tot_recs*buf_len;
            top_rec   = start + tot_recs;
            eh_watch_int(top_rec);
            eh_watch_int(buf_len);
            eh_watch_int(start);
            top_block = (top_rec + 1) / buf_len;
            eh_watch_int(top_block);

            //for ( i=start ; i<top_block ; i+=buf_len )
            for (i = start ; i < top_rec ; i += buf_len) {
                /* Write hydro records in blocks. */
                eh_watch_int(i);
                eh_watch_int(top_block);
                eh_watch_int(top_rec);
                all_recs = sed_hydrotrend_read_recs(*fp, i, buf_len, from, &error);
                eh_watch_ptr(all_recs);
                //big_recs = sed_hydro_array_eventize( all_recs , fraction , !just_events );
                big_recs = sed_hydro_array_eventize_conc(all_recs, 1045.);
                eh_watch_ptr(big_recs);
                //eh_watch_int( g_strv_length( all_recs ) );
                //eh_watch_int( g_strv_length( big_recs ) );

                write_data(fp_out, big_recs, out_type, to);
                //write_data( fp_out , all_recs , out_type , to );

                //eh_info( "Block start            : %d" , i                                        );
                //eh_info( "Total load             : %f" , sed_hydro_array_suspended_load(all_recs) );
                //eh_info( "Modeled load           : %f" , sed_hydro_array_suspended_load(big_recs) );
                //eh_info( "Number of events       : %d" , g_strv_length( (gchar**)big_recs )       );

                all_recs = sed_hydro_array_destroy(all_recs);
                big_recs = sed_hydro_array_destroy(big_recs);
            }

            fclose(*fp);
        }
    }

    eh_free(fp_in);
    fclose(fp_out);

    return EXIT_SUCCESS;
}

void
write_data(FILE* fp, Sed_hydro* a, gint type, gint order)
{
    if (a) {
        if (type == 0) {
            sed_hydro_array_write_hydrotrend_records_to_byte_order(fp, a, order);
        } else if (type == 1) {
            Sed_hydro* r;
            double     t;

            for (r = a ; *r ; r++)
                for (t = 0 ; t < sed_hydro_duration(*r) ; t++) {
                    fprintf(fp, "%f\n", sed_hydro_water_flux(*r));
                }

            //fprintf( fp , "%f\n" , sed_hydro_suspended_load(*r)/sed_hydro_duration(*r) );
        } else if (type == 2) {
            Sed_hydro* r;

            for (r = a ; *r ; r++)
                if (sed_hydro_is_hyperpycnal(*r)) {
                    sed_hydro_fprint_rec(fp, *r, NULL);
                }
        } else if (type == 3) {
            Sed_hydro* r;

            for (r = a ; *r ; r++)
                if (!sed_hydro_is_hyperpycnal(*r)) {
                    sed_hydro_fprint_rec(fp, *r, NULL);
                }
        } else {
            eh_require_not_reached();
        }
    }

    return;
}

#ifdef IGNORE_THIS

int
main(int argc, char* argv[])
{
    char* prop_vals[] = { "q", "qs", "v", "w", "d", "bed", NULL };
    int j;
    int n_recs, buf_len, n_sig;
    int start;
    int type;
    double time, total_time;
    int prop;
    gboolean verbose, use_buf;
    char* infile;
    Eh_args* args;
    Hydro_get_val_func get_val;
    Sed_hydro_file fp;
    Sed_hydro rec;
    GPtrArray* rec_array;

    args = eh_opts_init(argc, argv);

    if (eh_check_opts(args, NULL, NULL, help_msg) != 0) {
        eh_exit(-1);
    }

    total_time = eh_get_opt_dbl(args, "dt", 365.);
    n_recs  = eh_get_opt_int(args, "nrecs", 10);
    buf_len = eh_get_opt_int(args, "len", 365);
    n_sig   = eh_get_opt_int(args, "nsig", 5);
    start   = eh_get_opt_int(args, "start", 0);
    verbose = eh_get_opt_bool(args, "v", FALSE);
    use_buf = eh_get_opt_bool(args, "buf", FALSE);
    infile  = eh_get_opt_str(args, "in", "-");
    prop    = eh_get_opt_key(args, "prop", 0, prop_vals);

    switch (prop) {
        case 0:
            get_val = &sed_hydro_water_flux;
            break;

        case 1:
            get_val = &sed_hydro_suspended_flux;
            break;

        case 2:
            get_val = &sed_hydro_velocity;
            break;

        case 3:
            get_val = &sed_hydro_width;
            break;

        case 4:
            get_val = &sed_hydro_depth;
            break;

        case 5:
            get_val = &sed_hydro_bedload;
            break;
    }

    if (use_buf) {
        type = HYDRO_HYDROTREND | HYDRO_USE_BUFFER;
    } else {
        type = HYDRO_HYDROTREND;
    }

    if (verbose) {
        fprintf(stderr, "--- head ---\n");
        fprintf(stderr, "total time (days) : %f\n", total_time);
        fprintf(stderr, "n records    : %d\n", n_recs);
        fprintf(stderr, "buf length   : %d\n", buf_len);
        fprintf(stderr, "n sig events : %d\n", n_sig);
        fprintf(stderr, "start        : %d\n", start);
        fprintf(stderr, "property     : %s\n", prop_vals[prop]);
        fprintf(stderr, "buffer       : %d\n", use_buf);
    }

    fp = sed_hydro_file_new(infile, type, TRUE);
    sed_hydro_file_set_sig_values(fp, n_sig);
    sed_hydro_file_set_buffer_length(fp, buf_len);

    rec_array = g_ptr_array_new();
    fprintf(stdout, "%s\n", prop_vals[prop]);

    for (time = 0 ; time < total_time ;) {
        rec = sed_hydro_file_read_record(fp);

        if (sed_hydro_duration(rec) + time > total_time) {
            sed_hydro_set_duration(rec, total_time - time);
        }

        for (j = 0 ; j < sed_hydro_duration(rec) ; j++) {
            fprintf(stdout, "%f\n", (*get_val)(rec));
        }

        time += sed_hydro_duration(rec);
        g_ptr_array_add(rec_array, rec);
    }

    if (verbose) {
        double total = 0, total_qs = 0;

        for (j = 0 ; j < rec_array->len ; j++) {
            rec       = g_ptr_array_index(rec_array, j);
            total    += (*get_val)(rec) * sed_hydro_duration(rec);
            total_qs += sed_hydro_suspended_load(rec);
        }

        fprintf(stderr, "--- tail ---\n");
        fprintf(stderr, "n events         : %d\n", rec_array->len);
        fprintf(stderr, "total            : %f\n", total);
        fprintf(stderr, "total qs         : %f\n", total_qs);
    }

    g_ptr_array_free(rec_array, FALSE);

    sed_hydro_file_destroy(fp);

    return 0;
}

#endif

