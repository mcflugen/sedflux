#include "inflow.h"
#include "inflow_local.h"
#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>

GQuark
inflow_error_quark(void)
{
    return g_quark_from_static_string("inflow-error-quark");
}

static Inflow_param_st p;

static Eh_key_file_entry tmpl[] = {
    { "Length of basin", EH_ARG_DBL, &p.basin_len       },
    { "Bin spacing", EH_ARG_DBL, &p.dx              },
    { "Density of sea water", EH_ARG_DBL, &p.rho_sea_water   },
    { "Density of river water", EH_ARG_DBL, &p.rho_river_water },
    { "Removal rate constant", EH_ARG_DARRAY, &p.lambda, &p.n_grains },
    { "Equivalent grain diameter", EH_ARG_DARRAY, &p.size_equiv, &p.n_grains },
    { "Component grain diameter", EH_ARG_DARRAY, &p.size_comp, &p.n_grains },
    { "Fraction of each grain in river", EH_ARG_DARRAY, &p.grain_fraction, &p.n_grains },
    { "Fraction of flow occupied by each grain", EH_ARG_DARRAY, &p.flow_fraction, &p.n_grains },
    { "Bulk density", EH_ARG_DARRAY, &p.bulk_density, &p.n_grains },
    { "Grain density", EH_ARG_DARRAY, &p.grain_density, &p.n_grains },
    { "Distance from river to start deposition", EH_ARG_DBL, &p.dep_start       },
    { "Average grain size of bottom sediments", EH_ARG_DBL, &p.size_bottom     },
    { "Average bulk density of bottom sediments", EH_ARG_DBL, &p.rho_bottom      },
    { "Fraction of each grain size in bottom sediments", EH_ARG_DARRAY, &p.bottom_fraction, &p.n_grains },
    { "sua", EH_ARG_DBL, &p.sua             },
    { "sub", EH_ARG_DBL, &p.sub             },
    { "Entrainment constant, Ea", EH_ARG_DBL, &p.e_a             },
    { "Entrainment constant, Eb", EH_ARG_DBL, &p.e_b             },
    { "Coefficient of drag", EH_ARG_DBL, &p.c_drag          },
    { "Angle of internal friction", EH_ARG_DBL, &p.tan_phi         },
    { "Kinematic viscosity of clear water", EH_ARG_DBL, &p.mu_water        },
    { "Flood data file", EH_ARG_FILENAME, &p.flood_file    },
};

void
inflow_get_phe(Inflow_phe_query_st* query_data, Inflow_bottom_st* bed_data);

gboolean
inflow_wrapper(Inflow_bathy_st* b,
    Inflow_flood_st* f,
    Inflow_sediment_st* s,
    Inflow_const_st* c,
    double** deposition,
    double** erosion)
{
    FILE* fp_debug = g_getenv("INFLOW_DEBUG") ? stderr : NULL;

    return inflow(f->duration, b->x, b->slope,
            b->width, b->len, b->x[1] - b->x[0],
            c->dep_start, f->width, f->velocity,
            f->depth, f->q, f->fraction,
            s->size_equiv, s->lambda, s->bulk_density,
            s->grain_density, s->n_grains, c->rho_river_water,
            f->rho_flow, c, deposition,
            erosion, fp_debug);
}

Inflow_param_st*
inflow_scan_parameter_file(const gchar* file, GError** error)
{
    Inflow_param_st* p_new     = NULL;
    GError*          tmp_error = NULL;

    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    if (!file) {
        file = INFLOW_TEST_PARAM_FILE;
        eh_message("Reading parameter from default file: %s", file);
    }

    eh_key_file_scan_from_template(file, "INFLOW", tmpl, &tmp_error);

    if (!tmp_error) {
        p_new = eh_new(Inflow_param_st, 1);

        *p_new = p;

        p_new->basin_len      *= 1000;
        p_new->dep_start      *= 1000;
        p_new->mu_water       *= 1e-6;
        p_new->tan_phi         = tan(p.tan_phi * G_PI / 180.);

        eh_dbl_array_mult(p_new->lambda, p_new->n_grains, S_DAYS_PER_SECOND);
        eh_dbl_array_mult(p_new->size_equiv, p_new->n_grains, 1e-6);
        eh_dbl_array_mult(p_new->size_comp, p_new->n_grains, 1e-6);

        inflow_check_params(p_new, &tmp_error);
    }

    if (tmp_error) {
        eh_free(p_new);
        p_new = NULL;
        g_propagate_error(error, tmp_error);
    }

    return p_new;
}

Inflow_param_st*
inflow_check_params(Inflow_param_st* p, GError** error)
{
    eh_require(p);
    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    if (p) {
        gchar** err_s = NULL;

        eh_check_to_s(p->basin_len > 0.,      "Basin length positive", &err_s);
        eh_check_to_s(p->dx > 0.,      "Spacing positive", &err_s);
        eh_check_to_s(p->dx < p->basin_len,      "Spacing less than basin_length", &err_s);
        eh_check_to_s(p->basin_len / p->dx > 5,      "Not enough nodes", &err_s);
        eh_check_to_s(p->rho_sea_water >= p->rho_river_water,
            "Sea water density greater than river water", &err_s);

        eh_check_to_s(p->rho_river_water >= 1000.,
            "River water density greater than 1000 kg/m^3.", &err_s);
        eh_check_to_s(eh_dbl_array_each_ge(0., p->lambda, p->n_grains),
            "Removal rates positive", &err_s);

        eh_check_to_s(eh_dbl_array_cmp_ge(p->size_equiv, p->size_comp, p->n_grains),
            "Equivalent diameters >= component diameters", &err_s);
        eh_check_to_s(eh_dbl_array_each_ge(0., p->flow_fraction, p->n_grains),
            "Fraction of flow >= 0.", &err_s);
        eh_check_to_s(eh_dbl_array_each_le(1., p->flow_fraction, p->n_grains),
            "Fraction of flow <= 1.", &err_s);
        eh_check_to_s(eh_dbl_array_each_ge(p->rho_sea_water, p->bulk_density, p->n_grains),
            "Bulk density greater than sea water", &err_s);
        eh_check_to_s(eh_dbl_array_cmp_ge(p->grain_density, p->bulk_density, p->n_grains),
            "Grain density greater than bulk density", &err_s);
        eh_check_to_s(p->dep_start >= 0,           "Start of deposition greater than zero",
            &err_s);
        eh_check_to_s(p->dep_start < p->basin_len, "Start of deposition less than basin length",
            &err_s);
        eh_check_to_s(p->size_bottom > 0.,
            "Grain size of bottom sediments greater than zero", &err_s);
        eh_check_to_s(p->rho_bottom > p->rho_sea_water,
            "Bulk density of bottom sediments >= Density of sea water", &err_s);
        eh_check_to_s(eh_dbl_array_each_ge(0., p->bottom_fraction, p->n_grains),
            "Fraction of bottom sediment >= 0.", &err_s);
        eh_check_to_s(eh_dbl_array_each_le(1., p->bottom_fraction, p->n_grains),
            "Fraction of bottom sediment <= 1.", &err_s);

        eh_check_to_s(p->sua > 0.,                 "sua positive", &err_s);
        eh_check_to_s(p->sub > 0.,                 "sub positive", &err_s);
        eh_check_to_s(p->e_a > 0.,                 "Ea positive", &err_s);
        eh_check_to_s(p->e_b > 0.,                 "Eb positve", &err_s);
        eh_check_to_s(p->c_drag > 0.,              "Drag coefficient positive", &err_s);
        eh_check_to_s(p->tan_phi > 0.,            "Internal friction angle positive", &err_s);
        eh_check_to_s(p->tan_phi < 90.,            "Internal friction angle < 90 degrees",
            &err_s);
        eh_check_to_s(p->mu_water > 0.,            "Viscosity of water > 0", &err_s);

        if (err_s)
            eh_set_error_strv(error,
                INFLOW_ERROR,
                INFLOW_ERROR_BAD_PARAMETER, err_s);

        g_strfreev(err_s);
    }

    return p;
}

Inflow_bathy_st*
inflow_scan_bathy_file(const gchar* file, Inflow_param_st* p, GError** error)
{
    Inflow_bathy_st* b         = NULL;
    GError*          tmp_error = NULL;

    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    eh_require(p);

    if (!file) {
        file = INFLOW_TEST_BATHY_FILE;
        eh_message("Reading bathymetry from default file: %s", file);
    }

    {
        double** bathy;
        gint n_rows, n_cols;

        bathy = eh_dlm_read_swap(file, ",;", &n_rows, &n_cols, &tmp_error);

        eh_require(n_rows == 3);

        if (!tmp_error) {
            b = inflow_set_bathy_data(bathy, n_cols, p->dx, p->basin_len);
        } else {
            g_propagate_error(error, tmp_error);
        }

        eh_free_2(bathy);
    }

    return b;
}

Inflow_bathy_st*
inflow_update_bathy_data(Inflow_bathy_st* b, double** deposition, double** erosion,
    gint n_grains)
{

    if (b) {
        gint i, n;
        double sum;

        for (i = 0 ; i < b->len - 1 ; i++) {
            for (n = 0, sum = 0. ; n < n_grains ; n++) {
                sum += deposition[n][i] - erosion[n][i];
            }

            b->depth[i] += sum;
        }

        for (i = 0 ; i < b->len - 1 ; i++) {
            b->slope[i] = atan((b->depth[i + 1] - b->depth[i]) / (b->x[i + 1] - b->x[i]));
        }

        b->slope[b->len - 1] = b->slope[b->len - 2];
    }

    return b;

}

Inflow_bathy_st*
inflow_set_bathy_data(double** bathy, gint len, double dx, double basin_len)
{
    Inflow_bathy_st* b = NULL;

    if (bathy) {
        gint i;
        double x_0 = bathy[0][0];
        double x_1 = x_0 + basin_len;

        b    = eh_new(Inflow_bathy_st, 1);
        b->x = eh_uniform_array(x_0, x_1, dx, &(b->len));

        eh_require(b->len > 1);

        b->depth = eh_new(double, b->len);
        b->width = eh_new(double, b->len);
        b->slope = eh_new(double, b->len);

        interpolate(bathy[0], bathy[1], len, b->x, b->depth, b->len);
        interpolate(bathy[0], bathy[2], len, b->x, b->width, b->len);

        for (i = 0 ; i < b->len - 1 ; i++) {
            b->slope[i] = atan((b->depth[i + 1] - b->depth[i]) / (b->x[i + 1] - b->x[i]));
        }

        b->slope[b->len - 1] = b->slope[b->len - 2];

        for (i = 0 ; i < b->len ; i++) {
            b->x[i] -= x_0;
        }
    }

    return b;
}

Inflow_bathy_st*
inflow_destroy_bathy_data(Inflow_bathy_st* b)
{
    if (b) {
        eh_free(b->x);
        eh_free(b->depth);
        eh_free(b->width);
        eh_free(b->slope);
        eh_free(b);
    }

    return NULL;
}

Inflow_flood_st**
inflow_scan_flood_file(const gchar* file, Inflow_param_st* p, GError** error)
{
    Inflow_flood_st** river_data = NULL;

    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    if (!file) {
        file = INFLOW_TEST_FLOOD_FILE;
        eh_message("Reading flood data from default file: %s", file);
    }

    {
        GError*    tmp_error = NULL;
        Sed_hydro* river     = sed_hydro_scan(file, &tmp_error);
        gint       i, len;

        if (river) {
            for (len = 0 ; river[len] ; len++);

            river_data = eh_new0(Inflow_flood_st*, len + 1);

            for (i = 0 ; i < len && !tmp_error; i++) {
                river_data[i] = inflow_set_flood_data(river[i], p->rho_river_water);
            }

            river_data[len] = NULL;

            for (i = 0 ; river[i] ; i++) {
                sed_hydro_destroy(river[i]);
            }

            eh_free(river);
        } else {
            g_propagate_error(error, tmp_error);
        }
    }

    return river_data;
}

Inflow_flood_st*
inflow_set_flood_data(Sed_hydro h, double rho_river_water)
{
    Inflow_flood_st* r = NULL;

    {
        r = eh_new(Inflow_flood_st, 1);

        r->duration = sed_hydro_duration_in_seconds(h);
        r->width    = sed_hydro_width(h);
        r->depth    = sed_hydro_depth(h);
        r->velocity = sed_hydro_velocity(h);
        r->q        = sed_hydro_water_flux(h);
        r->rho_flow = sed_hydro_flow_density(h, rho_river_water);
        r->fraction = sed_hydro_fraction(h);
        r->n_grains = sed_hydro_size(h) - 1;
    }

    return r;
}

Inflow_flood_st*
inflow_destroy_flood_data(Inflow_flood_st* f)
{
    if (f) {
        eh_free(f->fraction);
        eh_free(f);
    }

    return NULL;
}

Inflow_sediment_st*
inflow_set_sediment_data(Inflow_param_st* p)
{
    Inflow_sediment_st* s = eh_new(Inflow_sediment_st, 1);
    gint n;

    s->size_equiv    = eh_dbl_array_dup(p->size_equiv, p->n_grains);
    s->lambda        = eh_dbl_array_dup(p->lambda, p->n_grains);
    s->bulk_density  = eh_dbl_array_dup(p->bulk_density, p->n_grains);
    s->grain_density = eh_dbl_array_dup(p->grain_density, p->n_grains);
    s->n_grains      = p->n_grains;

    /* Divide the lambdas by the equivalentHeights.  This is added
       to account for different grains occupying different portions
       of the flow height (ie sands mostly near the bottom, clays
       distributed evenly bottom to top).
    */
    for (n = 0 ; n < p->n_grains ; n++) {
        s->lambda[n] /= p->flow_fraction[n];
    }

    return s;
}

Inflow_const_st*
inflow_set_constant_data(Inflow_param_st* p)
{
    Inflow_const_st* c  = eh_new(Inflow_const_st, 1);
    Inflow_bottom_st* b = eh_new(Inflow_bottom_st, 1);

    c->e_a             = p->e_a;
    c->e_b             = p->e_b;
    c->sua             = p->sua;
    c->sub             = p->sub;
    c->c_drag          = p->c_drag;
    c->tan_phi         = p->tan_phi;
    c->mu_water        = p->mu_water;
    c->rho_river_water = p->rho_river_water;
    c->rho_sea_water   = p->rho_sea_water;
    c->channel_len     = p->channel_len;
    c->channel_width   = p->channel_width;
    //   c->day             = S_SECONDS_PER_DAY;
    c->dep_start       = p->dep_start;

    b->phe_bottom      = p->bottom_fraction;
    b->n_grains        = p->n_grains;
    c->get_phe_data    = b;
    c->get_phe         = (Inflow_query_func)inflow_get_phe;

    return c;
}

gint
inflow_write_output(const gchar* file,
    Inflow_bathy_st* b,
    double** deposit,
    gssize n_grains)
{
    gint bytes = 0;
    FILE* fp;

    if (file) {
        fp = eh_open_file(file, "w");
    } else {
        fp = stdout;
    }

    if (fp) {
        gint i, n;
        double sum;

        fprintf(fp, "# Inflow input/output bathymetry file.\n");
        fprintf(fp, "# Columns are :\n");
        fprintf(fp, "#    Position (m), Water Depth (m), Width (m)\n");

        for (i = 0 ; i < b->len ; i++) {
            for (n = 0, sum = 0 ; n < n_grains ; n++) {
                sum += deposit[n][i];
            }

            bytes += fprintf(fp, "%f %f %f\n", b->x[i], b->depth[i] + sum, b->width[i]);
            //         bytes += fprintf( fp , "%f %f %f\n" , b->x[i] , sum , b->width[i] );
        }
    }

    fclose(fp);

    return bytes;
}

void
inflow_get_phe(Inflow_phe_query_st* query_data, Inflow_bottom_st* bed_data)
{
    double* phe_out    = query_data->phe;
    double* phe_bottom = bed_data->phe_bottom;
    gint    n_grains   = bed_data->n_grains;

    memcpy(phe_out, phe_bottom, sizeof(double)*n_grains);

    return;
}

