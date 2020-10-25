#define BIO_PROCESS_NAME_S "bioturbation"
#define EH_LOG_DOMAIN BIO_PROCESS_NAME_S

#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "bio.h"

G_GNUC_INTERNAL Sed_cell*
bio_array_to_cell_array(Sed_cell* c_arr, double** data);
G_GNUC_INTERNAL double**
sed_array_to_bio_array(Sed_cell* c_arr, gint* n_grains, gint* n_layers);
G_GNUC_INTERNAL void
sed_column_bioturbate(Sed_column c, double depth, double k, double duration,
    Bio_method m);

#include <stdio.h>
#include <sed/sed_sedflux.h>
#include "bio.h"

typedef struct {
    Eh_input_val k;
    Eh_input_val r;
    Eh_input_val depth;
    Bio_method   method;
}
Bio_param_t;

static double bio_k;
static double bio_r;

static const GOptionEntry bio_args[] = {
    { "bio-k", 0, 0, G_OPTION_ARG_DOUBLE, &bio_k, "Set bioturbation diffusion coefficient (cm^2/y)", "VAL" },
    { "bio-r", 0, 0, G_OPTION_ARG_DOUBLE, &bio_r, "Set bioturbation conveyor rate (cm/d)", "VAL" },
    { "bio-depth", 0, 0, G_OPTION_ARG_DOUBLE, &bio_r, "Set depth of bioturbation (m)", "VAL" },
    { NULL }
};

GOptionGroup*
bio_get_option_group(void)
{
    GOptionGroup* g = NULL;

    g = g_option_group_new("bioturbation", "Bioturbation options",
            "Show Bioturbation options", NULL, NULL);
    g_option_group_add_entries(g, bio_args);

    return g;
}

Sed_process_info
bio_run(Sed_process proc, Sed_cube p)
{
    Bio_param_t* data = (Bio_param_t*)sed_process_user_data(proc);
    Sed_process_info info = SED_EMPTY_INFO;

    eh_require(data);
    eh_require(data->depth);
    eh_require(data->k || data->r);

    if (data) {
        gint   i;
        gint   len   = sed_cube_size(p);
        double dt    = sed_cube_time_step_in_seconds(p);
        double time  = sed_cube_age_in_years(p);
        double depth = eh_input_val_eval(data->depth, time);
        double k;
        double r;
        double val;

        // Convert k from cm^2/y to m^2/s. Convert r from cm/d to m/s
        if (data->k) {
            k = eh_input_val_eval(data->k, time) * 1e-4 / S_SECONDS_PER_YEAR;
        }

        if (data->r) {
            r = eh_input_val_eval(data->r, time) * 1e-2 / S_SECONDS_PER_DAY;
        }

        switch (data->method) {
            case BIO_METHOD_DIFFUSION:
                val = k;
                break;

            case BIO_METHOD_CONVEYOR :
                val = r;
                break;

            default:
                eh_require_not_reached();
        }

        eh_require(val > 0);
        eh_require(depth > 0);
        eh_require(dt > 0);

        //      EH_MEM_LEAK_START

        for (i = 0 ; i < len ; i++) {
            sed_column_bioturbate(sed_cube_col(p, i), depth, val, dt, data->method);
        }

        //      EH_MEM_LEAK_END_WARN
    }

    return info;
}

/*
typedef enum
{
   BIO_KEY_MODEL ,
   BIO_KEY_DEPTH ,
   BIO_KEY_K ,
   BIO_KEY_R
};

static const gchar* bio_default_val[] =    { [BIO_KEY_MODEL] = "diffusion" ,
                                             [BIO_KEY_DEPTH] = ".1"        ,
                                             [BIO_KEY_K]     = "50."       ,
                                             [BIO_KEY_R]     = ".001" };
static const gchar* bio_req_label[] =      { [BIO_KEY_MODEL] = "Bioturbation model" ,
                                             [BIO_KEY_DEPTH] = "Depth of bioturbation" };
static const gchar* bio_diff_req_label[] = { [BIO_KEY_K]     = "Diffusion coefficient" };
static const gchar* bio_conv_req_label[] = { [BIO_KEY_R]     = "Conveyor rate"         };
*/

#define BIO_KEY_DEPTH   "depth of bioturbation"
#define BIO_KEY_MODEL   "bioturbation model"
#define BIO_KEY_K       "diffusion coefficient"
#define BIO_KEY_R       "conveyor rate"

static const gchar* bio_diff_req_label[] = { BIO_KEY_K, NULL };
static const gchar* bio_conv_req_label[] = { BIO_KEY_R, NULL };
static const gchar* bio_req_label     [] = { BIO_KEY_DEPTH, BIO_KEY_MODEL, NULL };

/*
gint
bio_fprint_kvf( FILE* fp )
{
   gint n = 0;

   eh_require(fp);

   if ( fp )
   {
      n += fprintf( fp , "%s : %s\n" , bio_req_label[BIO_KEY_DEPTH] , bio_default_val[BIO_KEY_DEPTH] );
      n += fprintf( fp , "%s : %s\n" , bio_req_label[BIO_KEY_MODEL] , bio_default_val[BIO_KEY_MODEL] );
      n += fprintf( fp , "%s : %s\n" , bio_req_label[BIO_KEY_K]     , bio_default_val[BIO_KEY_K]     );
   }

   return n;
}
*/

gboolean
bio_init(Sed_process p, Eh_symbol_table t, GError** error)
{
    Bio_param_t* data    = sed_process_new_user_data(p, Bio_param_t);
    GError*      tmp_err = NULL;
    gboolean     is_ok   = TRUE;

    eh_return_val_if_fail(error == NULL || *error == NULL, FALSE);
    eh_require(t);
    eh_require(p);

    data->method = BIO_METHOD_UNKNOWN;

    if (eh_symbol_table_require_labels(t, bio_req_label, &tmp_err)) {
        //gchar** err_s   = NULL;
        gchar*  model_s = eh_symbol_table_lookup(t, BIO_KEY_MODEL);

        data->depth = eh_symbol_table_input_value(t, BIO_KEY_DEPTH, &tmp_err);

        if (g_ascii_strcasecmp(model_s, "DIFFUSION") == 0) {
            data->method = BIO_METHOD_DIFFUSION;
        } else if (g_ascii_strcasecmp(model_s, "CONVEYOR") == 0) {
            data->method = BIO_METHOD_CONVEYOR;
        } else
            g_set_error(&tmp_err, BIO_ERROR, BIO_ERROR_BAD_ALGORITHM,
                "invalid bioturbation model (diffusion or conveyor): %s", model_s);

        if (!tmp_err
            && data->method == BIO_METHOD_DIFFUSION
            && eh_symbol_table_require_labels(t, bio_diff_req_label, &tmp_err)) {
            data->k = eh_symbol_table_input_value(t, BIO_KEY_K, &tmp_err);
        }

        if (!tmp_err
            && data->method == BIO_METHOD_CONVEYOR
            && eh_symbol_table_require_labels(t, bio_conv_req_label, &tmp_err)) {
            data->r = eh_symbol_table_input_value(t, BIO_KEY_R, &tmp_err);
        }
    }

    if (tmp_err) {
        g_propagate_error(error, tmp_err);
        is_ok = FALSE;
    }

    return is_ok;
}

gboolean
bio_destroy(Sed_process p)
{
    if (p) {
        Bio_param_t* data = (Bio_param_t*)sed_process_user_data(p);

        if (data) {
            eh_input_val_destroy(data->k);
            eh_input_val_destroy(data->depth);
            eh_free(data);
        }
    }

    return TRUE;
}

void
sed_column_bioturbate(Sed_column c, double depth, double k, double duration,
    Bio_method m)
{
    //   EH_MEM_LEAK_START

    eh_require(c);
    eh_require(k > 0);
    eh_require(duration > 0);

    if (c && depth >= 0) {
        double    mass_in  = sed_column_mass(c);
        double    mass_out = 0;
        double    z        = sed_column_top_height(c) - depth;
        Sed_cell* top      = sed_column_extract_cells_above(c, z);

        if (top) {
            double   dz       = sed_column_z_res(c);
            gint     n_layers = g_strv_length((gchar**)top);

            if (n_layers > 2) {
                double**  data = NULL;
                double*   t    = eh_new(double, n_layers);
                gint      i;

                for (i = 0 ; i < n_layers ; i++) {
                    t[i] = sed_cell_size(top[i]);
                    /*
                                   if ( !sed_cell_is_valid(top[i]) )
                                      sed_cell_fprint( stderr , top[i] );
                    */
                }

                {
                    double sum = eh_dbl_array_sum(t, n_layers);

                    if (sum > .1 && !eh_compare_dbl(sum, .1, 1e-12)) {
                        eh_watch_dbl(sum - .1);
                    }
                }

                switch (m) {
                    case BIO_METHOD_DIFFUSION:
                        data = bio_diffuse_layers(t, n_layers, dz, k, duration);
                        break;

                    case BIO_METHOD_CONVEYOR :
                        data = bio_conveyor_layers(t, n_layers, dz, k, duration);
                        break;

                    default:
                        eh_require_not_reached();
                }

                if (data) {
                    Sed_cell* new_top = bio_array_to_cell_array(top, data);

                    sed_column_stack_cells_loc(c, new_top);

                    eh_free(new_top);
                    g_strfreev((gchar**)data);
                } else {
                    sed_column_stack_cells_loc(c, top);
                    eh_free(top);
                    top = NULL;
                }

                eh_free(t);
            } else {
                sed_column_stack_cells_loc(c, top);
                eh_free(top);
                top = NULL;
            }

            mass_out = sed_column_mass(c);

            if (!eh_compare_dbl(mass_in, mass_out, 1e-2)) {
                eh_require_not_reached();
                eh_watch_dbl(mass_in);
                eh_watch_dbl(mass_out);
                eh_watch_int(n_layers);
                eh_watch_ptr(top);
            }

            top = sed_cell_array_free(top);
        }

    }

    //   EH_MEM_LEAK_END_WARN
}

double**
sed_array_to_bio_array(Sed_cell* col, gint* n_grains, gint* n_layers)
{
    double** data = NULL;

    eh_require(col);
    eh_require(n_grains);
    eh_require(n_layers);

    if (col) {
        gint i, n;

        *n_grains = sed_sediment_env_n_types();
        *n_layers = g_strv_length((gchar**)col);

        for (n = 0 ; n < *n_grains ; n++)
            for (i = 0 ; i < *n_layers ; i++) {
                data[n][i] = sed_cell_nth_amount(col[i], n);
            }
    }

    return data;
}

Sed_cell*
bio_array_to_cell_array(Sed_cell* c_arr, double** data)
{
    Sed_cell* out_arr = NULL;

    if (data && c_arr) {
        gint new_len  = g_strv_length((gchar**)data);
        gint orig_len = g_strv_length((gchar**)c_arr);
        gint i;
        gint j;
        double mass_in, mass_out;

        mass_in = sed_cell_array_mass(c_arr);

        out_arr = eh_new0(Sed_cell, new_len + 1);

        for (i = 0 ; i < new_len ; i++) {
            out_arr[i] = sed_cell_new_env();

            for (j = 0 ; j < orig_len ; j++)
                if (data[i][j] > 1e-12) {
                    /*
                                   if ( !sed_cell_is_valid(c_arr[j]) )
                                      sed_cell_fprint( stderr , c_arr[j] );
                    */
                    sed_cell_resize(c_arr[j], data[i][j]);
                    /*
                                   if ( !sed_cell_is_valid(c_arr[j]) )
                                   {
                                      eh_watch_dbl( data[i][j] );
                                      sed_cell_fprint( stderr , c_arr[j] );
                                   }
                    */
                    sed_cell_add(out_arr[i], c_arr[j]);
                }

            /*
                        if ( sed_cell_is_empty( out_arr[i] ) || sed_cell_is_clear( out_arr[i] ) || !sed_cell_is_valid(out_arr[i] ) )
                        {
                           sed_cell_fprint(stderr , out_arr[i] );
                           sed_cell_fprint(stderr , c_arr[0] );
            fprintf( stderr , "Thickness = %g\n" , sed_cell_size(c_arr[0]) );
            eh_watch_int( i );
            eh_watch_int( j );
                     for ( i=0 ; i<new_len ; i++ )
                     {
                        for ( j=0 ; j<orig_len ; j++ )
                           fprintf( stderr , "%f " , data[i][j] );
                        fprintf( stderr , "\n" );
                     }
                        }
            */
        }

        out_arr = sed_cell_array_delete_empty(out_arr);

        mass_out = sed_cell_array_mass(out_arr);

        if (!eh_compare_dbl(mass_in, mass_out, 1e-2)) {
            eh_require_not_reached();

            for (i = 0 ; i < new_len ; i++) {
                for (j = 0 ; j < orig_len ; j++) {
                    fprintf(stderr, "%f ", data[i][j]);
                }

                fprintf(stderr, "\n");
            }

            for (i = 0 ; i < new_len ; i++) {
                sed_cell_fprint(stderr, out_arr[i]);
            }

            eh_watch_dbl(mass_in);
            eh_watch_dbl(mass_out);
            eh_watch_int(g_strv_length((gchar**)c_arr));
            eh_watch_int(g_strv_length((gchar**)out_arr));
            eh_exit(0);
        }

    }

    return out_arr;
}

/*
Sed_cell*
bio_array_to_cell_array( Sed_cell* c_arr , double** data , gint n_grains , gint n_layers )
{
   if ( data && c_arr )
   {
      gint i, n;
      double* t = eh_new( double , n_grains );

      eh_require( n_layers==g_strv_length( (gchar**)c_arr )    );
      eh_require( n_grains==sed_sediment_env_n_types() );

      for ( i=0 ; i<n_layers ; i++ )
      {
         for ( n=0 ; n<n_grains ; n++ )
            t[n] = data[n][i];

         sed_cell_set_amount( c_arr[i] , t );
      }

      eh_free( t );
   }

   return c_arr;
}
*/
/*
void
sed_diffuse_array( Sed_cell* arr , double k , double duration )
{
   {

      for ( n=0 ; n<n_grains ; n++ )
      {
         for ( i=0 ; i<len ; i++ )
         {
            dh[i] = sed_cell_nth_amount(arr[i+1],n)
                  - sed_cell_nth_amount(arr[i]  ,n);
            q[i]  = k*dh[i]/dz;
         }
      }
   }
}
*/
