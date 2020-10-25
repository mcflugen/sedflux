#include "sakura.h"
#include "sakura_local.h"
#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>

GQuark
sakura_error_quark(void)
{
    return g_quark_from_static_string("sakura-error-quark");
}

static Sakura_param_st p;

static Eh_key_file_entry tmpl[] = {
    // { "Length of basin"                                 , EH_ARG_DBL    , &p.basin_len       } , // unused
    { "Bin spacing", EH_ARG_DBL, &p.dx              },
    { "Time step", EH_ARG_DBL, &p.dt              },
    // { "Output time step"                                , EH_ARG_DBL    , &p.out_dt          } , // unused
    // { "Maximum time"                                    , EH_ARG_DBL    , &p.max_t           } , // unused
    { "Density of sea water", EH_ARG_DBL, &p.rho_sea_water   },
    { "Density of river water", EH_ARG_DBL, &p.rho_river_water },
    { "Removal rate constant", EH_ARG_DARRAY, &p.lambda, &p.n_grains },
    // { "Equivalent grain diameter"                       , EH_ARG_DARRAY , &p.size_equiv      , &p.n_grains } , // unused
    // { "Component grain diameter"                        , EH_ARG_DARRAY , &p.size_comp       , &p.n_grains } , // unused
    // { "Fraction of each grain in river"                 , EH_ARG_DARRAY , &p.grain_fraction  , &p.n_grains } , // unused
    { "Fraction of flow occupied by each grain", EH_ARG_DARRAY, &p.flow_fraction, &p.n_grains },
    { "Bulk density", EH_ARG_DARRAY, &p.bulk_density, &p.n_grains },
    { "Grain density", EH_ARG_DARRAY, &p.grain_density, &p.n_grains },
    { "Distance from river to start deposition", EH_ARG_DBL, &p.dep_start       },
    // { "Average grain size of bottom sediments"          , EH_ARG_DBL    , &p.size_bottom     } , // unused
    // { "Average bulk density of bottom sediments"        , EH_ARG_DBL    , &p.rho_bottom      } , // unused
    { "Fraction of each grain size in bottom sediments", EH_ARG_DARRAY, &p.bottom_fraction, &p.n_grains },
    { "sua", EH_ARG_DBL, &p.sua             },
    { "sub", EH_ARG_DBL, &p.sub             },
    { "Entrainment constant, Ea", EH_ARG_DBL, &p.e_a             },
    { "Entrainment constant, Eb", EH_ARG_DBL, &p.e_b             },
    { "Coefficient of drag", EH_ARG_DBL, &p.c_drag          },
    { "Angle of internal friction", EH_ARG_DBL, &p.tan_phi         },
    { "Kinematic viscosity of clear water", EH_ARG_DBL, &p.mu_water        },
    // { "Flood data file"                                 , EH_ARG_FILENAME , &p.flood_file    } , // unused
    { NULL }
};

//void sakura_get_phe( Sakura_phe_query_st* query_data , Sakura_bottom_st* bed_data );

double**
sakura_wrapper(Sakura_bathy_st*    b,
    Sakura_flood_st*    f,
    Sakura_sediment_st* s,
    Sakura_const_st*    c,
    gint* n_grains,
    gint* len)
{
    double** deposit = NULL;
    //gboolean is_ok;

    eh_require(b);
    eh_require(f);
    eh_require(s);
    eh_require(c);
    eh_require(n_grains);
    eh_require(len);

    {
        //FILE*   fp_debug  = g_getenv("SAKURA_DEBUG")?stderr:NULL;
        //double  basin_len = b->x[b->len-1] - b->x[0];
        //double  dx        = b->dx;
        double* init_u    = eh_new(double, 2);
        double* init_c    = eh_new(double, 2);

        init_u[0] = f->velocity;
        init_u[1] = -1.;
        init_c[0] = f->rho_flow;
        init_c[1] = -1.;

        eh_require(b->x);
        eh_require(b->depth);
        eh_require(b->width);
        eh_require(init_u);
        eh_require(init_c);
        eh_require(s->lambda);
        eh_require(s->u_settling);
        //eh_require( s->reynolds_no   );
        eh_require(s->grain_density);
        eh_require(s->bulk_density);
        eh_require(f->fraction);

        /*
              sakura( dx                  , c->dt            , basin_len      ,
                      b->len              , s->n_grains      , b->x           ,
                      b->depth            , b->width         , init_u         ,
                      init_c              , s->lambda        , s->u_settling  ,
                      s->reynolds_no      , s->grain_density , f->depth       ,
                      f->duration         , c->dep_start     , f->fraction    ,
                      NULL                , s->bulk_density  , c->out_dt      ,
                      c                   , deposition       , NULL );
        */
        deposit =
            sakura(f->velocity, f->rho_flow, f->depth, f->fraction,
                c->dt, f->duration,
                b->x, b->depth, b->width, b->len,
                s->grain_density, s->bulk_density, s->u_settling, s->n_grains,
                c);

        *n_grains = s->n_grains;
        *len      = b->len;

        eh_free(init_u);
        eh_free(init_c);
    }

    return deposit;
    /*
       return sakura( f->duration         , b->x           , b->slope           ,
                      b->width            , b->len         , b->x[1]-b->x[0]    ,
                      c->dep_start        , f->width       , f->velocity        ,
                      f->depth            , f->q           , f->fraction        ,
                      s->size_equiv       , s->lambda      , s->bulk_density    ,
                      s->grain_density    , s->n_grains    , c->rho_river_water ,
                      f->rho_flow         , c              , deposition         ,
                      erosion             , fp_debug );
    */
}

Sakura_param_st*
sakura_scan_parameter_file(const gchar* file, GError** error)
{
    Sakura_param_st* p_new     = NULL;
    GError*          tmp_error = NULL;

    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    if (!file) {
        file = SAKURA_TEST_PARAM_FILE;
        eh_message("Reading parameter from default file: %s", file);
    }

    eh_key_file_scan_from_template(file, "SAKURA", tmpl, &tmp_error);

    if (!tmp_error) {
        p_new = eh_new(Sakura_param_st, 1);

        *p_new = p;

        //p_new->basin_len      *= 1000;
        p_new->dep_start      *= 1000;
        p_new->mu_water       *= 1e-6;
        p_new->tan_phi         = tan(p.tan_phi * G_PI / 180.);

        //      eh_dbl_array_mult( p_new->lambda     , p_new->n_grains , S_DAYS_PER_SECOND*0.1 );
        eh_dbl_array_mult(p_new->lambda, p_new->n_grains, S_DAYS_PER_SECOND);
        //eh_dbl_array_mult( p_new->size_equiv , p_new->n_grains , 1e-6                  );
        //eh_dbl_array_mult( p_new->size_comp  , p_new->n_grains , 1e-6                  );

        sakura_check_params(p_new, &tmp_error);
    }

    if (tmp_error) {
        eh_free(p_new);
        p_new = NULL;
        g_propagate_error(error, tmp_error);
    }

    return p_new;
}

Sakura_param_st*
sakura_check_params(Sakura_param_st* p, GError** error)
{
    eh_require(p);
    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    if (p) {
        gchar** err_s = NULL;

        //eh_check_to_s( p->basin_len > 0.    ,      "Basin length positive" , &err_s );
        eh_check_to_s(p->dx > 0.,      "Spacing positive", &err_s);
        //eh_check_to_s( p->dx < p->basin_len ,      "Spacing less than basin_length" , &err_s );
        eh_check_to_s(p->dt > 0.,      "Time step positive", &err_s);
        //eh_check_to_s( p->out_dt > 0.       ,      "Output time step positive" , &err_s );
        //eh_check_to_s( p->max_t > 0.        ,      "Maximum run time positive" , &err_s );
        //eh_check_to_s( p->basin_len/p->dx>5 ,      "Not enough nodes" , &err_s );
        eh_check_to_s(p->rho_sea_water >= p->rho_river_water,
            "Sea water density greater than river water", &err_s);

        eh_check_to_s(p->rho_river_water >= 1000.,
            "River water density greater than 1000 kg/m^3.", &err_s);
        eh_check_to_s(eh_dbl_array_each_ge(0., p->lambda, p->n_grains),
            "Removal rates positive", &err_s);

        //eh_check_to_s( eh_dbl_array_cmp_ge( p->size_equiv , p->size_comp , p->n_grains ) ,
        //                                           "Equivalent diameters >= component diameters" , &err_s );
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
        //eh_check_to_s( p->dep_start<p->basin_len , "Start of deposition less than basin length" , &err_s  );
        //eh_check_to_s( p->size_bottom>0. ,         "Grain size of bottom sediments greater than zero" , &err_s );
        //eh_check_to_s( p->rho_bottom>p->rho_sea_water ,
        //                                           "Bulk density of bottom sediments >= Density of sea water" , &err_s );
        //eh_check_to_s( eh_dbl_array_each_ge( 0. , p->bottom_fraction , p->n_grains ) ,
        //                                           "Fraction of bottom sediment >= 0." , &err_s );
        //eh_check_to_s( eh_dbl_array_each_le( 1. , p->bottom_fraction , p->n_grains ) ,
        //                                           "Fraction of bottom sediment <= 1." , &err_s );

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
                SAKURA_ERROR,
                SAKURA_ERROR_BAD_PARAMETER, err_s);

        g_strfreev(err_s);
    }

    return p;
}

Sakura_bathy_st*
sakura_scan_bathy_file(const gchar* file, Sakura_param_st* p, GError** error)
{
    Sakura_bathy_st* b         = NULL;
    GError*          tmp_error = NULL;

    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    eh_require(p);

    if (!file) {
        file = SAKURA_TEST_BATHY_FILE;
        eh_message("Reading bathymetry from default file: %s", file);
    }

    {
        double** bathy;
        gint     n_rows;
        gint     n_cols;

        bathy = eh_dlm_read_swap(file, ",;", &n_rows, &n_cols, &tmp_error);

        eh_require(n_rows == 3);

        if (!tmp_error)
            //b = sakura_set_bathy_data( bathy , n_cols , p->dx , p->n_grains , p->basin_len );
        {
            b = sakura_set_bathy_data(bathy, n_cols, p->dx, p->n_grains);
        } else {
            g_propagate_error(error, tmp_error);
        }

        eh_free_2(bathy);
    }

    return b;
}

Sakura_bathy_st*
sakura_update_bathy_data(Sakura_bathy_st* b, double** deposition, double** erosion,
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

Sakura_bathy_st*
sakura_set_bathy_data(double** bathy, gint len, double dx, gint n_grains)
{
    Sakura_bathy_st* b = NULL;

    eh_require(bathy);
    eh_require(bathy[0]);
    eh_require(bathy[1]);
    eh_require(bathy[2]);
    eh_require(len >= 2);
    eh_require(dx  > 0.);
    eh_require(n_grains > 0.);
    //eh_require( basin_len <= bathy[0][len-1] - bathy[0][0] );

    if (bathy) {
        gint   i;
        //double x_0 = bathy[0][0] + dx/2.;
        double x_0 = bathy[0][0];
        //      double x_1 = bathy[0][0] + basin_len;
        double x_1 = bathy[0][len - 1];

        b     = eh_new(Sakura_bathy_st, 1);

        b->x  = eh_uniform_array(x_0, x_1, dx, &(b->len));
        b->dx = dx;

        b->n_grains = n_grains;

        eh_require(b->len > 1);

        b->depth = eh_new(double, b->len);
        b->width = eh_new(double, b->len);
        b->slope = eh_new(double, b->len);

        b->dep   = eh_new_2(double, n_grains, b->len);

        interpolate(bathy[0], bathy[1], len, b->x, b->depth, b->len);
        interpolate(bathy[0], bathy[2], len, b->x, b->width, b->len);

        for (i = 0 ; i < b->len - 1 ; i++) {
            b->slope[i] = atan((b->depth[i + 1] - b->depth[i]) / (b->x[i + 1] - b->x[i]));
        }

        b->slope[b->len - 1] = b->slope[b->len - 2];

        //for ( i=0 ; i<b->len ; i++ )
        //   b->x[i] -= bathy[0][0];
    }

    return b;
}

Sakura_bathy_st*
sakura_new_bathy_data(gint n_grains, gint len)
{
    Sakura_bathy_st* b = NULL;

    if (len > 0) {
        b = eh_new(Sakura_bathy_st, 1);

        b->x     = eh_new(double, len);
        b->depth = eh_new(double, len);
        b->width = eh_new(double, len);
        b->slope = eh_new(double, len);

        b->dep   = eh_new_2(double, n_grains, len);

        b->n_grains = n_grains;
        b->len      = len;
        b->dx       = 0;
    }

    return b;
}

Sakura_bathy_st*
sakura_copy_bathy_data(Sakura_bathy_st* d, const Sakura_bathy_st* s)
{
    eh_require(s);

    if (s) {
        gint n;

        if (!d) {
            d = sakura_new_bathy_data(s->n_grains, s->len);
        }

        eh_require(d);
        eh_require(d->len == s->len);

        d->dx = s->dx;

        memcpy(d->x, s->x, sizeof(double)*s->len);
        memcpy(d->depth, s->depth, sizeof(double)*s->len);
        memcpy(d->width, s->width, sizeof(double)*s->len);
        memcpy(d->slope, s->slope, sizeof(double)*s->len);

        for (n = 0 ; n < s->n_grains ; n++) {
            memcpy(d->dep[n], s->dep[n], sizeof(double)*s->len);
        }
    }

    return d;
}

Sakura_bathy_st*
sakura_destroy_bathy_data(Sakura_bathy_st* b)
{
    if (b) {
        eh_free(b->x);
        eh_free(b->depth);
        eh_free(b->width);
        eh_free(b->slope);
        eh_free_2(b->dep);
        eh_free(b);
    }

    return NULL;
}

Sakura_flood_st**
sakura_scan_flood_file(const gchar* file, Sakura_param_st* p, GError** error)
{
    Sakura_flood_st** river_data = NULL;

    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    if (!file) {
        file = SAKURA_TEST_FLOOD_FILE;
        eh_message("Reading flood data from default file: %s", file);
    }

    {
        GError*    tmp_error = NULL;
        Sed_hydro* river     = sed_hydro_scan(file, &tmp_error);
        gint       i, len;

        if (river) {
            for (len = 0 ; river[len] ; len++);

            river_data = eh_new0(Sakura_flood_st*, len + 1);

            for (i = 0 ; i < len && !tmp_error; i++) {
                river_data[i] = sakura_set_flood_data(river[i], p->rho_river_water);
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

Sakura_flood_st*
sakura_set_flood_data(Sed_hydro h, double rho_river_water)
{
    Sakura_flood_st* r = NULL;

    {
        r = eh_new(Sakura_flood_st, 1);

        r->duration = sed_hydro_duration_in_seconds(h);
        r->width    = sed_hydro_width(h);
        r->depth    = sed_hydro_depth(h);
        r->velocity = sed_hydro_velocity(h);
        r->q        = sed_hydro_water_flux(h);
        r->rho_flow = sed_hydro_flow_density(h, rho_river_water) - rho_river_water;
        r->fraction = sed_hydro_fraction(h);
        r->n_grains = sed_hydro_size(h);
    }

    return r;
}

Sakura_flood_st*
sakura_sed_set_flood_data(Sed_hydro h, double rho_river_water)
{
    Sakura_flood_st* r = NULL;

    {

        r = eh_new(Sakura_flood_st, 1);

        r->duration = sed_hydro_duration_in_seconds(h);
        r->width    = sed_hydro_width(h);
        r->depth    = sed_hydro_depth(h);
        r->velocity = sed_hydro_velocity(h);
        r->q        = sed_hydro_water_flux(h);
        r->rho_flow = sed_hydro_flow_density(h, rho_river_water) - rho_river_water;
        r->n_grains = sed_hydro_size(h) + 1;

        {
            gint    n;
            double* f = sed_hydro_fraction(h); /* Suspended fractions */

            r->fraction = eh_new(double, r->n_grains);
            r->fraction[0] = 0;

            for (n = 1 ; n < r->n_grains ; n++) {
                r->fraction[n] = f[n - 1];
            }

            eh_free(f);
        }
    }

    return r;
}


Sakura_flood_st*
sakura_destroy_flood_data(Sakura_flood_st* f)
{
    if (f) {
        eh_free(f->fraction);
        eh_free(f);
    }

    return NULL;
}

Sakura_sediment_st*
sakura_destroy_sediment_data(Sakura_sediment_st* s)
{
    if (s) {
        eh_free(s->size_equiv);
        eh_free(s->lambda);
        eh_free(s->bulk_density);
        eh_free(s->grain_density);
        eh_free(s->u_settling);
        eh_free(s->reynolds_no);
        eh_free(s);
    }

    return NULL;
}

Sakura_sediment_st*
sakura_set_sediment_data(Sakura_param_st* p)
{
    Sakura_sediment_st* s = eh_new(Sakura_sediment_st, 1);
    gint n;

    //   s->size_equiv    = eh_dbl_array_dup( p->size_equiv    , p->n_grains );
    s->lambda        = eh_dbl_array_dup(p->lambda, p->n_grains);
    s->bulk_density  = eh_dbl_array_dup(p->bulk_density, p->n_grains);
    s->grain_density = eh_dbl_array_dup(p->grain_density, p->n_grains);
    s->u_settling    = eh_new(double, p->n_grains);
    //   s->reynolds_no   = eh_new( double , p->n_grains );
    s->n_grains      = p->n_grains;

    /* Divide the lambdas by the equivalentHeights.  This is added
       to account for different grains occupying different portions
       of the flow height (ie sands mostly near the bottom, clays
       distributed evenly bottom to top).
    */
    for (n = 0 ; n < p->n_grains ; n++) {
        s->lambda[n]      /= p->flow_fraction[n];
        //s->lambda[n] *= 10.;
        s->u_settling[n]   = sed_removal_rate_to_settling_velocity(s->lambda[n] *
                S_SECONDS_PER_DAY)
            * S_DAYS_PER_SECOND;

        //s->u_settling[n]   = sakura_settling_velocity( s->grain_density[n] , s->size_equiv[n] ,
        //                                            p->rho_river_water  , p->mu_water );
        //      s->reynolds_no[n]  = sakura_reynolds_number  ( s->grain_density[n] , s->size_equiv[n] ,
        //                                                  p->rho_river_water  , p->mu_water );
        if (TRUE) {
            eh_message("Settling velocity (cm/s): %f", s->u_settling[n] * 100.);
        }
    }

    return s;
}

void
sakura_set_width(Sakura_bathy_st* bathy_data,
    double           river_width,
    double           spreading_angle)
{
    gint   i;
    //double dx = bathy_data->x[1] - bathy_data->x[0];
    //double flow_width;

    // Create a spreading angle.
    bathy_data->width[0] = river_width;

    for (i = 1 ; i < bathy_data->len ; i++) {
        bathy_data->width[i] = river_width;
        /*
              flow_width = bathy_data->width[i-1] + spreading_angle*dx;

              if ( flow_width < bathy_data->width[i] ) bathy_data->width[i] = flow_width;
              else                                     break;
        */
    }

    return;
}
#include <math.h>

double
sakura_settling_velocity(double rho_grain, double equiv_dia, double rho_river_water,
    double mu_river_water)
{
    double u = 0.;

    eh_require(rho_grain      > 0.);
    eh_require(equiv_dia      > 0.);
    eh_require(rho_river_water > 0.);
    eh_require(mu_river_water > 0.);

    {
        double Rden  = rho_grain / rho_river_water;
        double Dstar = sed_gravity() * Rden * pow(equiv_dia, 3.0) / pow(mu_river_water, 2.0);
        double Wstar = -3.76715
            + (1.92944 * log10(Dstar))
            - (0.09815 * pow(log10(Dstar), 2.0))
            - (0.00575 * pow(log10(Dstar), 3.0))
            + (0.00056 * pow(log10(Dstar), 4.0));

        Wstar = pow(10.0, Wstar);

        u = pow((Rden * G * mu_river_water * Wstar), 0.33333);

        eh_watch_dbl(u * 100);
        u = 0.005;
        eh_watch_dbl(u * 100);

    }

    return u;
}

double
sakura_reynolds_number(double rho_grain, double equiv_dia, double rho_river_water,
    double mu_river_water)
{
    double r = 0.;

    eh_require(rho_grain      > 0.);
    eh_require(equiv_dia      > 0.);
    eh_require(rho_river_water > 0.);
    eh_require(mu_river_water > 0.);

    {
        double Rden  = rho_grain / rho_river_water;
        double Dstar = sed_gravity() * Rden * pow(equiv_dia, 3.0) / pow(mu_river_water, 2.0);
        double Wstar = -3.76715
            + (1.92944 * log10(Dstar))
            - (0.09815 * pow(log10(Dstar), 2.0))
            - (0.00575 * pow(log10(Dstar), 3.0))
            + (0.00056 * pow(log10(Dstar), 4.0));

        Wstar = pow(10.0, Wstar);

        r = sqrt(Rden * G * pow(equiv_dia, 3.0)) / mu_river_water;
    }

    return r;
}

Sakura_const_st*
sakura_set_constant_data(Sakura_param_st* p, Sakura_bathy_st* b)
{
    Sakura_const_st* c    = eh_new(Sakura_const_st, 1);
    Sakura_arch_st*  arch = eh_new(Sakura_arch_st, 1);

    c->dt              = p->dt;

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

    c->get_phe         = (Sakura_phe_func)sakura_get_phe;
    c->add             = (Sakura_add_func)sakura_add;
    c->remove          = (Sakura_add_func)sakura_remove;
    c->get_depth       = (Sakura_get_func)sakura_get_depth;

    arch->b            = b;
    arch->phe          = p->bottom_fraction;
    arch->n_grains     = p->n_grains;

    c->get_phe_data    = arch;
    c->depth_data      = arch;
    c->add_data        = arch;
    c->remove_data     = arch;

    return c;
}

Sakura_const_st*
sakura_set_constant_output_data(Sakura_const_st* c, const gchar* file, Sakura_var* id,
    gint dt)
{
    eh_require(c);

    if (c) {
        if (file && id && dt > 0) {
            c->data_fp  = eh_fopen(file, "w");
            c->data_id  = id;
            c->data_int = dt;
        } else {
            c->data_fp  = NULL;
            c->data_id  = NULL;
            c->data_int = -1;
        }
    }

    return c;
}

gint
sakura_write_data(const gchar*     file,
    Eh_dbl_grid      deposit)
{
    gint  n = 0;

    eh_require(deposit);

    if (file && deposit) {
        gint  i, j;
        FILE* fp = eh_open_file(file, "a");
        double** d        = eh_dbl_grid_data(deposit);
        gint     n_grains = eh_grid_n_x(deposit);
        gint     len      = eh_grid_n_y(deposit);
        double   width = 30000;
        double   dx    = 100.;

        for (j = 0 ; j < n_grains ; j++) {
            n += fprintf(fp, "%f", d[j][0] / (width * dx));

            for (i = 1 ; i < len ; i++) {
                n += fprintf(fp, "; %f", d[j][i] / (width * dx));
            }

            n += fprintf(fp, "\n");
        }

        fclose(fp);
    }

    return n;
}

gint
sakura_write_output(const gchar*     file,
    Sakura_bathy_st* b,
    double**         deposit,
    gssize           n_grains)
{
    gint  bytes = 0;
    FILE* fp;

    if (file) {
        fp = eh_open_file(file, "w");
    } else {
        fp = stdout;
    }

    if (fp) {
        gint i, n;
        double sum;

        fprintf(fp, "# Sakura input/output bathymetry file.\n");
        fprintf(fp, "# Columns are :\n");
        fprintf(fp, "#    Position (m), Water Depth (m), Width (m)\n");

        for (i = 0 ; i < b->len ; i++) {
            for (n = 0, sum = 0 ; n < n_grains ; n++) {
                sum += deposit[n][i];
            }

            bytes += fprintf(fp, "%f; %f; %f\n", b->x[i], b->depth[i] + sum, b->width[i]);
            //         bytes += fprintf( fp , "%f %f %f\n" , b->x[i] , sum , b->width[i] );
        }
    }

    fclose(fp);

    return bytes;
}

#if defined( EXCLUDE_THIS )
/** Get the grain size distribution of bottom sediments.

Get the fractions of each grain type of bottom sediments from a
Sed_cube.  This function is intended to be used within another program that
needs to communicate with the sedflux architecture but is otherwise separate
from sedflux.

Note that the member, eroded_depth may be changed to reflect the actual amount
of bottom sediment available to be eroded.  That is, we may be trying to erode
more sediment than is actually present.

In this case, the data pointer should point to a Sakura_get_phe_t structure
that contains the grain size distribution and the number of grains.  This
information is simply copied to the location pointed to by phe.

@param query_data  The fraction of each grain type in the bottom sediments.
@param bed_data    A structure that contains the necessary data for the function to
                   retreive the grain type fracitons.

@return            A pointer to the array of grain type fractions.

*/
void
sakura_get_phe(Sakura_phe_query_st* query_data, Sakura_bottom_st* bed_data)
{
    double* phe_out    = query_data->phe;
    double* phe_bottom = bed_data->phe_bottom;
    gint    n_grains   = bed_data->n_grains;

    memcpy(phe_out, phe_bottom, sizeof(double)*n_grains);

    return;
}

void
sakura_remove(Sakura_erode_query_st remove_query, gpointer data)
{
    double remove = EH_STRUCT_MEMBER(Sak_erode_query_t, remove_query, dh);
    int    i      = EH_STRUCT_MEMBER(Sak_erode_query_t, remove_query, i);

    double* x       = EH_STRUCT_MEMBER(Sak_bathy_t, data, x);
    double* depth   = EH_STRUCT_MEMBER(Sak_bathy_t, data, depth);
    double* slope   = EH_STRUCT_MEMBER(Sak_bathy_t, data, slope);
    int     n_nodes = EH_STRUCT_MEMBER(Sak_bathy_t, data, n_nodes);

    remove = (remove > 0) ? remove : -remove;
    depth[i] -= remove;

    /*
       if ( i==n_nodes-1 )
          slope[i]   = atan( (depth[i]-depth[i-1])/(x[i]-x[i-1]));
       else
          slope[i]   = atan( (depth[i+1]-depth[i])/(x[i+1]-x[i]));

       if ( i>0 )
          slope[i-1] = atan( (depth[i]-depth[i-1])/(x[i]-x[i-1]));
    */
    return;
}

void
sakura_add(gpointer add_query, gpointer data)
{
    double add = EH_STRUCT_MEMBER(Sak_add_query_t, add_query, dh);
    int    i   = EH_STRUCT_MEMBER(Sak_add_query_t, add_query, i);
    double* x       = EH_STRUCT_MEMBER(Sak_bathy_t, data, x);
    double* depth   = EH_STRUCT_MEMBER(Sak_bathy_t, data, depth);
    double* slope   = EH_STRUCT_MEMBER(Sak_bathy_t, data, slope);
    int     n_nodes = EH_STRUCT_MEMBER(Sak_bathy_t, data, n_nodes);

    add = (add > 0) ? add : -add;
    depth[i] += add;

    /*
       if ( i==n_nodes-1 )
          slope[i]   = atan( (depth[i]-depth[i-1])/(x[i]-x[i-1]));
       else
          slope[i]   = atan( (depth[i+1]-depth[i])/(x[i+1]-x[i]));

       if ( i>0 )
          slope[i-1] = atan( (depth[i]-depth[i-1])/(x[i]-x[i-1]));
    */

    return;
}

void
sakura_get_depth(gpointer depth_query, gpointer data)
{
    int    i   = EH_STRUCT_MEMBER(Sak_depth_query_t, depth_query, i);
    double* depth   = EH_STRUCT_MEMBER(Sak_bathy_t, data, depth);

    EH_STRUCT_MEMBER(Sak_depth_query_t, depth_query, depth) = depth[i];

    return;
}

void
sakura_set_depth(gpointer depth_query, gpointer data)
{
    int    i         = EH_STRUCT_MEMBER(Sak_depth_query_t,
            depth_query,
            i);
    double new_depth = EH_STRUCT_MEMBER(Sak_depth_query_t,
            depth_query,
            depth);
    double* depth   = EH_STRUCT_MEMBER(Sak_bathy_t, data, depth);

    depth[i] = new_depth;

    return;
}
#endif

/** Get the grain size distribution of bottom sediments.

Get the fractions of each grain type of bottom sediments from a
Sed_cube.  This function is intended to be used within another program that
needs to communicate with the sedflux architecture but is otherwise separate
from sedflux.

Note that the member, eroded_depth may be changed to reflect the actual amount
of bottom sediment available to be eroded.  That is, we may be trying to erode
more sediment than is actually present.

In this case, the data pointer should point to a Sakura_get_phe_t structure
that contains the grain size distribution and the number of grains.  This
information is simply copied to the location pointed to by phe.

\param   data       Pointer to a Sakura_arch_st
\param   x          Location to query architecture for grain size info
\param   phe_data   Pointer to Sakura_phe_st
*/
void
sakura_get_phe(Sakura_arch_st* data, double x, Sakura_phe_st* phe_data)
{
    eh_require(data);
    eh_require(phe_data);

    if (data && phe_data) {
        double* phe_out    = phe_data->phe;
        gint    n_grains   = phe_data->n_grains;
        double* phe_bottom = data->phe;

        eh_require(phe_bottom);
        eh_require(n_grains > 0);
        eh_require(n_grains == data->n_grains);

        if (!phe_out) {
            phe_out = eh_new(double, n_grains);
        }

        memcpy(phe_out, phe_bottom, sizeof(double)*n_grains);

        phe_data->phe = phe_out;
    }

    return;
}

double
sakura_add(Sakura_arch_st* data, double x, Sakura_cell_st* s)
{
    double vol_add = 0.;

    eh_require(data);
    eh_require(data->b);
    eh_require(s);

    if (data && s && s->t > 0) {
        Sakura_bathy_st* b     = data->b;
        gint             len   = b->len;
        double*          depth = b->depth;
        gint             ind   = (x - b->x[0]) / b->dx;
        double           dh;
        const double     width = 30000.;

        //eh_make_note( "Setting width to 30km" );

        eh_require(ind >= 0);
        eh_require(ind < len);
        eh_require(depth);

        //      vol_add     = s->t;

        //dh          = s->t/(b->dx*b->width[ind]);
        dh          = s->t / (b->dx * width);

        if (depth[ind] + dh > 0) {
            dh = -depth[ind];
        }

        if (dh < 0) {
            dh = 0;
        }

        depth[ind] += fabs(dh);

        b->dep[s->id][ind] += fabs(dh);

        //vol_add     = dh*b->dx*b->width[ind];
        vol_add     = dh * b->dx * width;
    }

    return vol_add;
}

double
sakura_remove(Sakura_arch_st* data, double x, Sakura_cell_st* s)
{
    double vol_rem = 0.;

    eh_require(data);
    eh_require(data->b);
    eh_require(s);

    if (data && s && s->t > 0) {
        Sakura_bathy_st* b     = data->b;
        gint             len   = b->len;
        double*          depth = b->depth;
        gint             ind   = (x - b->x[0]) / (b->dx);
        double           dh;

        eh_require(ind >= 0);
        eh_require(ind < len);
        eh_require(depth);

        vol_rem     = s->t;
        dh          = vol_rem / (b->dx * b->width[ind]);
        depth[ind] -= fabs(dh);

        b->dep[s->id][ind] -= fabs(dh);
    }

    return vol_rem;
}

double
sakura_get_depth(Sakura_arch_st* data, double x)
{
    double depth_val = 0.;

    eh_require(data);
    eh_require(data->b);

    if (data) {
        Sakura_bathy_st* b     = data->b;
        gint             len   = b->len;
        double*          depth = b->depth;
        //gint             ind   = (b->x[0]-x)/b->dx;
        gint             ind   = (x - b->x[0]) / b->dx;

        eh_require(depth);
        eh_require(ind >= 0);
        eh_require(ind < len);

        depth_val = depth[ind];
    }

    return depth_val;
}

