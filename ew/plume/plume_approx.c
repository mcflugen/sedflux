#include <glib.h>

#include "plume_approx.h"

double
plume_establishment_centerline_inv_at(double x, double l)
{
    return exp(-l * x);
}

double
plume_established_centerline_inv_at(double x, double l)
{
    return   exp(-l * x)
        * pow(PLUME_XA / x, PLUME_P1)
        * exp(-(2 * l / (3 * sqrt(PLUME_XA)) * (pow(x, 1.5) - pow(PLUME_XA, 1.5)) * PLUME_F2));
}

double
plume_establishment_inv_at(double x, double s, double l)
{
    return   exp(-l * x);
}

double
plume_plug_inventroy_at(double x, double s, double l)
{
    return exp(-l * x);
}

double
plume_established_inv_at(double x, double s, double l)
{
    return   exp(-l * x)
        * exp(-pow(PLUME_M1 * s, 2))
        * pow(PLUME_XA / x, PLUME_P1)
        * exp(-(2 * l / (3 * sqrt(PLUME_XA))
                * (pow(x, 1.5) - pow(PLUME_XA, 1.5))
                * exp(pow(PLUME_M2 * s, 2))
                * PLUME_F2));
}

double
plume_inv_far(double x, gpointer data)
{
    double l = *(double*)data;
    return plume_established_centerline_inv_at(x, l);
}

double
plume_inv_near(double x, gpointer data)
{
    double l = *(double*)data;
    return plume_establishment_centerline_inv_at(x, l);
}

double
plume_centerline_inv_at(double x, double l)
{
    double inv;

    eh_return_val_if_fail(x >= 0, 0.);
    eh_require(l > 0);

    if (x < PLUME_XA) {
        inv = plume_establishment_centerline_inv_at(x, l);
    } else {
        inv = plume_established_centerline_inv_at(x, l);
    }

    return inv;
}

double*
plume_centerline_inv_nd(double* inv, double* x, gssize len, double l)
{
    eh_return_val_if_fail(x, NULL);
    eh_return_val_if_fail(len > 0, NULL);

    {
        gssize i;

        if (!inv) {
            inv = eh_new(double, len);
        }

        for (i = 0 ; i < len ; i++) {
            inv[i] = plume_centerline_inv_at(x[i], l);
        }
    }

    return inv;
}

double*
plume_centerline_inv(double* inv, double* x, gssize len,
    double  l, double  i_0, Sed_hydro r)
{
    eh_return_val_if_fail(x, NULL);
    eh_return_val_if_fail(len > 0, NULL);
    eh_return_val_if_fail(r, NULL);
    eh_return_val_if_fail(i_0 > 0, NULL);

    if (!inv) {
        inv = eh_new(double, len);
    }

    {
        gssize i;
        double b0  = sed_hydro_width(r);
        double u0  = sed_hydro_velocity(r);

        l *= b0 / (u0 * S_SECONDS_PER_DAY);

        for (i = 0 ; i < len && x[i] / b0 < PLUME_XA ; i++) {
            inv[i] = i_0 * plume_establishment_centerline_inv_at(x[i] / b0, l);
        }

        for (; i < len ; i++) {
            inv[i] = i_0 * plume_established_centerline_inv_at(x[i] / b0, l);
        }
    }

    return inv;
}

double
plume_inv_nd_helper(double y, gpointer data)
{
    double x = ((double*)data)[0];
    double l = ((double*)data)[1];
    return plume_established_inv_at(x, PLUME_SIGMA * y / x, l);
}

double
plume_width_averaged_inv_at(double x, double dy, double l)
{
    double data[3];

    data[0] = x;
    data[1] = l;

    return eh_integrate_with_data(plume_inv_nd_helper, 0.01, dy, data);
}

double*
plume_width_averaged_inv(double* inv, double* x, gssize len,
    double  l, double i_0, double dy,
    Sed_hydro r)
{
    if (!inv) {
        inv = eh_new(double, len);
    }

    {
        gssize i;
        double b0 = sed_hydro_width(r);
        double u0 = sed_hydro_velocity(r);

        l *= b0 / (u0 * S_SECONDS_PER_YEAR);
        dy /= b0;

        /*
              for ( i=0 ; i<len && x[i]/b0<PLUME_XA ; i++ )
                 inv[i] = i_0 * plume_establishment_width_averaged_inv_at( x[i]/b0 , dy , l );
        */
        for (i = 0 ; i < len ; i++) {
            inv[i] = i_0 * plume_width_averaged_inv_at(x[i] / b0, dy, l);
        }
    }

    return inv;
}

Sed_cell_grid
plume_width_averaged_deposit(Sed_cell_grid g, Sed_hydro r, Sed_sediment s, double dy)
{
    double* x;

    eh_return_val_if_fail(g, NULL);
    eh_return_val_if_fail(r, NULL);

    // Set up non-dimensional distances
    {
        gssize len = eh_grid_n_y(g);

        x = (double*)g_memdup(eh_grid_y(g), sizeof(double) * len);

        eh_dbl_array_mult(x, len, 1. / sed_hydro_width(r));
    }

    // Integrate the deposit thickness over the grid
    {
        gssize i, n;
        double i_0, rho, l;
        gssize n_grains = sed_sediment_n_types(s);
        gssize len      = eh_grid_n_y(g);
        double* t       = eh_new0(double, n_grains);
        double* dep     = eh_new(double, len);
        double dt       = sed_hydro_duration(r);

        for (n = 1 ; n < n_grains ; n++) {
            l   = plume_non_dim_lambda(sed_type_lambda(sed_sediment_type(s, n)), r);
            rho = sed_type_rho_sat(sed_sediment_type(s, n));
            i_0 = sed_hydro_nth_concentration(r, n) * sed_hydro_depth(r);

            plume_width_averaged_deposit_nd(dep, x, len, l);

            for (i = 0 ; i < len ; i++) {
                t[n] = dep[i] * i_0 / rho * dt / dy;
                sed_cell_add_amount(sed_cell_grid_val(g, 0, i), t);
            }

            eh_dbl_array_mult(t, n_grains, 0.);
        }

        eh_free(t);
        eh_free(dep);
    }

    eh_free(x);

    return g;
}

double
plume_width_averaged_inventory_at_helper(double x, gpointer data)
{
    double dy = ((double*)data)[0];
    double l  = ((double*)data)[1];
    return plume_width_averaged_inv_at(x, dy, l);
}

double*
plume_width_averaged_deposit_nd(double* dep, double* x, gssize len, double l)
{
    eh_return_val_if_fail(x, NULL);
    eh_return_val_if_fail(len > 0, NULL);
    eh_return_val_if_fail(l > 0, NULL);

    if (!dep) {
        dep = eh_new(double, len);
    }

    {
        gssize i;
        double data[2];

        data[0] = 0.;
        data[1] = l;

        for (i = 0 ; i < len ; i++) {
            x[i] += PLUME_XA;
        }

        for (i = 1 ; i < len ; i++) {
            if (x[i] < PLUME_XA) {
                data[0] = .5 + (.25 - .5 / PLUME_XA) * x[i];
            } else {
                data[0] = .25 * x[i];
            }

            dep[i - 1] = eh_integrate_with_data(plume_width_averaged_inventory_at_helper, x[i - 1],
                    x[i], data)
                / (x[i] - x[i - 1]) * 2;
        }
    }

    return dep;
}

double*
plume_centerline_deposit_nd(double* dep, double* x, gssize len, double l)
{
    eh_return_val_if_fail(x, NULL);
    eh_return_val_if_fail(len > 0, NULL);
    eh_return_val_if_fail(l > 0, NULL);

    if (!dep) {
        dep = eh_new(double, len);
    }

    {
        gssize i;

        for (i = 1 ; i < len && x[i] < PLUME_XA ; i++)
            dep[i - 1] = eh_integrate_with_data(plume_inv_near, x[i - 1], x[i], &l)
                / (x[i] - x[i - 1]);

        if (i < len) {
            dep[i - 1]  = (eh_integrate_with_data(plume_inv_near, x[i - 1], PLUME_XA, &l)
                    + eh_integrate_with_data(plume_inv_far, PLUME_XA, x[i], &l))
                / (x[i] - x[i - 1]);

            for (i += 1 ; i < len ; i++)
                dep[i - 1] = eh_integrate_with_data(plume_inv_far, x[i - 1], x[i], &l)
                    / (x[i] - x[i - 1]);
        }
    }

    return dep;
}

Sed_cell_grid
plume_centerline_deposit(Sed_cell_grid g, Sed_hydro r, Sed_sediment s)
{
    double* x;

    eh_return_val_if_fail(g, NULL);
    eh_return_val_if_fail(r, NULL);

    // Set up non-dimensional distances
    {
        gssize len = eh_grid_n_y(g);

        x = (double*)g_memdup(eh_grid_y(g), sizeof(double) * len);

        eh_dbl_array_mult(x, len, 1. / sed_hydro_width(r));

    }

    // Integrate the deposit thickness over the grid
    {
        gssize i, n;
        double i_0, rho, l;
        gssize n_grains = sed_sediment_n_types(s);
        gssize len      = eh_grid_n_y(g);
        double* t       = eh_new0(double, n_grains);
        double* dep     = eh_new(double, len);
        double dt       = sed_hydro_duration(r);

        for (n = 1 ; n < n_grains ; n++) {
            l   = plume_non_dim_lambda(sed_type_lambda(sed_sediment_type(s, n)), r);
            rho = sed_type_rho_sat(sed_sediment_type(s, n));
            i_0 = sed_hydro_nth_concentration(r, n) * sed_hydro_depth(r);

            plume_centerline_deposit_nd(dep, x, len, l);

            for (i = 0 ; i < len ; i++) {
                t[n] = dep[i] * i_0 / rho * dt;
                sed_cell_add_amount(sed_cell_grid_val(g, 0, i), t);
            }

            eh_dbl_array_mult(t, n_grains, 0.);
        }

        eh_free(t);
        eh_free(dep);
    }

    eh_free(x);

    return g;
}

double*
plume_inv_nd(double* dep, double* x, double* s,  gssize len, double l)
{
    eh_return_val_if_fail(x, NULL);
    eh_return_val_if_fail(len > 0, NULL);
    eh_return_val_if_fail(l > 0, NULL);

    if (!dep) {
        dep = eh_new(double, len);
    }

    {
        gssize i;

        for (i = 1 ; i < len && x[i] < PLUME_XA ; i++)
            dep[i - 1] = eh_integrate_with_data(plume_inv_near, x[i - 1], x[i], &l)
                / (x[i] - x[i - 1]);

        if (i < len) {
            dep[i - 1]  = (eh_integrate_with_data(plume_inv_near, x[i - 1], PLUME_XA, &l)
                    + eh_integrate_with_data(plume_inv_far, PLUME_XA, x[i], &l))
                / (x[i] - x[i - 1]);

            for (i += 1 ; i < len ; i++)
                dep[i - 1] = eh_integrate_with_data(plume_inv_far, x[i - 1], x[i], &l)
                    / (x[i] - x[i - 1]);
        }
    }

    return dep;
}

double*
plume_inv(double* inv, double* x, double* y, gssize len,
    double  l, double  i_0, Sed_hydro r)
{
    eh_return_val_if_fail(x, NULL);
    eh_return_val_if_fail(y, NULL);
    eh_return_val_if_fail(len > 0, NULL);
    eh_return_val_if_fail(r, NULL);
    eh_return_val_if_fail(i_0 > 0, NULL);

    if (!inv) {
        inv = eh_new(double, len);
    }

    {
        gssize i;
        double b0  = sed_hydro_width(r);
        double u0  = sed_hydro_velocity(r);

        l *= b0 / (u0 * S_SECONDS_PER_DAY);

        for (i = 0 ; i < len && x[i] / b0 < PLUME_XA ; i++) {
            if (x[i] < PLUME_XA * b0) {
                inv[i] = i_0 * plume_establishment_inv_at(x[i] / b0, PLUME_SIGMA * y[i] / x[i], l);
            } else {
                inv[i] = i_0 * plume_established_inv_at(x[i] / b0, PLUME_SIGMA * y[i] / x[i], l);
            }
        }
    }

    return inv;
}

double
plume_non_dim_distance(double x, Sed_hydro r)
{
    return x / sed_hydro_width(r);
}

double
plume_non_dim_lambda(double l, Sed_hydro r)
{
    return l / S_SECONDS_PER_DAY * sed_hydro_width(r) / sed_hydro_velocity(r);
}

double
plume_v_bar_established(double x, double y)
{
    double s = PLUME_SIGMA * y / x;
    return sqrt(PLUME_XA / x) / PLUME_SIGMA * (s * exp(-pow(s,
                    2.)) - .25 * EH_SQRT_PI * erf(s));
}

double
plume_v_bar_establishment(double x, double y)
{
    double w = PLUME_SIGMA / x * (y - .5 * (1. - x / PLUME_XA));
    //   return plume_v_bar_established(x,y);
    return exp(-pow(w, 2)) * (w / PLUME_SIGMA - .5 / PLUME_XA) + .5 / PLUME_XA *
        (1 - G_SQRT2 * erf(w));
}

double
plume_v_bar_plug(double x, double y)
{
    //   return plume_v_bar_established(x,y);
    return 0.;
}

double
plume_u_bar_plug(double x, double y)
{
    return 1.;
}

double
plume_u_bar_establishment(double x, double y)
{
    if (eh_compare_dbl(x, 0., 1e-12)) {
        return 0;
    } else {
        double w = PLUME_SIGMA / x * (y - .5 * (1. - x / PLUME_XA));
        return exp(- pow(w, 2));
    }
}

double
plume_u_bar_established(double x, double y)
{
    return sqrt(PLUME_XA / x) * exp(-pow(PLUME_SIGMA * y / x, 2));
}

double
plume_k_bar_established(double x, double y)
{
    double s = PLUME_SIGMA * y / x;
    double temp;

    if (eh_compare_dbl(s, 0., 1e-12)) {
        temp = 2 / EH_SQRT_PI;
    } else {
        temp = erf(s) / s;
    }

    return (.5 / pow(PLUME_SIGMA, 2) * sqrt(PLUME_XA / x) * x) * (.25 / EH_SQRT_PI) * temp;
}

double
plume_k_bar_plug(double x, double y)
{
    return 0.;
}

double
plume_k_bar_establishment(double x, double y)
{
    return plume_k_bar_established(x, y);
}

double
plume_k_bar_dy_plug(double x, double y)
{
    return 0.;
}

double
plume_k_bar_dy_established(double x, double y)
{
    double s = PLUME_SIGMA * y / x;
    double temp;

    if (eh_compare_dbl(s, 0., 1e-12)) {
        temp = 0.;
    } else {
        temp = (2.*s / EH_SQRT_PI * exp(-pow(s, 2.)) - erf(s)) / (s * s);
    }

    return (.5 / PLUME_SIGMA * sqrt(PLUME_XA / x)) * (.25 / EH_SQRT_PI)
        * temp;
}

double
plume_k_bar_dy_establishment(double x, double y)
{
    return plume_k_bar_dy_established(x, y);
}

double*
plume_u_bar_at_x(double x, double dy, gint* n_y)
{
    double* u;

    eh_require(n_y);

    eh_return_val_if_fail(x >= 0, NULL);
    eh_return_val_if_fail(dy > 0, NULL);

    {
        gssize i;
        double y_max = plume_half_width(x);
        double* y = eh_uniform_array(0, y_max, dy, n_y);
        gssize len = *n_y;

        u = eh_new(double, len);

        if (x <= PLUME_XA) {
            double y_plug          = plume_plug_width(x);
            double y_establishment = plume_establishment_width(x);

            eh_require(y_max >= y_plug);
            eh_require(y_max >= y_establishment);

            for (i = 0 ; i < len && y[i] <= y_plug ; i++) {
                u[i] = plume_u_bar_plug(x, y[i]);
            }

            for (; i < len && y[i] <= y_establishment ; i++) {
                u[i] = plume_u_bar_establishment(x, y[i]);
            }
        } else {
            double y_established = plume_established_width(x);

            eh_require(y_max >= y_established);

            for (i = 0 ; i < len && y[i] <= y_established   ; i++) {
                u[i] = plume_u_bar_established(x, y[i]);
            }
        }

        eh_free(y);
    }

    return u;
}

double*
plume_v_bar_at_x(double x, double dy, gint* n_y)
{
    double* v;

    eh_require(n_y);

    eh_return_val_if_fail(x >= 0, NULL);
    eh_return_val_if_fail(dy > 0, NULL);

    {
        gssize i;
        double y_max = plume_half_width(x);
        double* y = eh_uniform_array(0, y_max, dy, n_y);
        gssize len = *n_y;

        v = eh_new(double, len);

        if (x <= PLUME_XA) {
            double y_plug          = plume_plug_width(x);
            double y_establishment = plume_establishment_width(x);

            eh_require(y_max >= y_plug);
            eh_require(y_max >= y_establishment);

            for (i = 0 ; i < len && y[i] <= y_plug ; i++) {
                v[i] = plume_v_bar_plug(x, y[i]);
            }

            for (; i < len && y[i] <= y_establishment ; i++) {
                v[i] = plume_v_bar_establishment(x, y[i]);
            }
        } else {
            double y_established = plume_established_width(x);

            eh_require(y_max >= y_established);

            for (i = 0 ; i < len && y[i] <= y_established   ; i++) {
                v[i] = plume_v_bar_established(x, y[i]);
            }
        }

        eh_free(y);
    }

    return v;
}

double*
plume_k_bar_at_x(double x, double dy, gint* n_y)
{
    double* k;

    eh_require(n_y);

    eh_return_val_if_fail(x >= 0, NULL);
    eh_return_val_if_fail(dy > 0, NULL);

    {
        gssize i;
        double y_max = plume_half_width(x);
        double* y = eh_uniform_array(0, y_max, dy, n_y);
        gssize len = *n_y;

        k = eh_new(double, len);

        if (x <= PLUME_XA) {
            double y_plug          = plume_plug_width(x);
            double y_establishment = plume_establishment_width(x);

            eh_require(y_max >= y_plug);
            eh_require(y_max >= y_establishment);

            for (i = 0 ; i < len && y[i] <= y_plug ; i++) {
                k[i] = plume_k_bar_plug(x, y[i]);
            }

            for (; i < len && y[i] <= y_establishment ; i++) {
                k[i] = plume_k_bar_establishment(x, y[i]);
            }
        } else {
            double y_established = plume_established_width(x);

            eh_require(y_max >= y_established);

            for (i = 0 ; i < len && y[i] <= y_established   ; i++) {
                k[i] = plume_k_bar_established(x, y[i]);
            }
        }

        eh_free(y);
    }

    return k;
}

double*
plume_k_bar_dy_at_x(double x, double dy, gint* n_y)
{
    double* k;

    eh_require(n_y);

    eh_return_val_if_fail(x >= 0, NULL);
    eh_return_val_if_fail(dy > 0, NULL);

    {
        gssize i;
        double y_max = plume_half_width(x);
        double* y = eh_uniform_array(0, y_max, dy, n_y);
        gssize len = *n_y;

        k = eh_new(double, len);

        if (x <= PLUME_XA) {
            double y_plug          = plume_plug_width(x);
            double y_establishment = plume_establishment_width(x);

            eh_require(y_max >= y_plug);
            eh_require(y_max >= y_establishment);

            for (i = 0 ; i < len && y[i] <= y_plug ; i++) {
                k[i] = plume_k_bar_dy_plug(x, y[i]);
            }

            for (; i < len && y[i] <= y_establishment ; i++) {
                k[i] = plume_k_bar_dy_establishment(x, y[i]);
            }
        } else {
            double y_established = plume_established_width(x);

            eh_require(y_max >= y_established);

            for (i = 0 ; i < len && y[i] <= y_established   ; i++) {
                k[i] = plume_k_bar_dy_established(x, y[i]);
            }
        }

        eh_free(y);
    }

    return k;
}

double**
plume_i_bar(double* x, gssize n_x, double l, gssize* n_y, double dy)
{
    double** i_bar;

    eh_return_val_if_fail(x, NULL);
    eh_return_val_if_fail(n_x > 0, NULL);
    eh_require(l > 0);
    eh_require(n_y);

    {
        double y_max = plume_half_width(x[n_x - 1]);
        gssize len = y_max / dy + 1;

        i_bar = eh_new_2(double, n_x, len);

        *n_y = len;
    }

    {
        gssize j;
        double y_max = plume_half_width(x[0]);
        gssize len = y_max / dy;

        for (j = 0 ; j < len ; j++) {
            i_bar[0][j] = 1;
        }
    }

    {
        gint i;
        gint len;
        double* i_bar_at_x;

        for (i = 1 ; i < n_x ; i++) {
            i_bar_at_x = plume_i_bar_at_x(x[i], x[i] - x[i - 1], dy, &len, l, i_bar[i - 1]);

            eh_require(len <= (*n_y));

            g_memmove(i_bar[i], i_bar_at_x, sizeof(double)*len);

            eh_free(i_bar_at_x);
        }

        /*
              // Estimate how much is lost.
              x_end = x[len-1];
              do
              {
                 i_bar_at_x = plume_i_bar_at_x( x_end , .5 , dy , &len , l , i_bar_last[i-1] );

                 i_bar_sum = eh_dbl_array_sum( i_bar_at_x , len );
                 i_bar_lost += i_bar_sum;

                 eh_free( i_bar_at_x );
              }
              while ( i_bar_sum>.01 );
        */
    }

    return i_bar;
}
/*
double plume_i_bar_total( double* i_bar_0 , gssize len , double x_0 , double x_1 , double y_0 , double y_1 , double l )
{
   double** i_bar;

   {
      gssize i;
      double y_max = plume_half_width( x_1 );
      gssize len = y_max / dy + 1;

      i_bar = eh_new_2( double , n_x , len );

      *n_y = len;
   }

   {
      gssize j;
      double y_max = plume_half_width( x[0] );
      gssize len = y_max / dy;

      for ( j=0 ; j<len ; j++ )
         i_bar[0][j] = 1;
   }

   {
      gssize i;
      gssize len;
      double* i_bar_at_x;

      for ( i=1 ; i<n_x ; i++ )
      {
         i_bar_at_x = plume_i_bar_at_x( x[i] , x[i]-x[i-1] , dy , &len , l , i_bar[i-1] );

         eh_require( len<=(*n_y) );

         g_memmove( i_bar[i] , i_bar_at_x , sizeof(double)*len );

         eh_free( i_bar_at_x );
      }

      // Estimate how much is lost.
      x_end = x[len-1];
      do
      {
         i_bar_at_x = plume_i_bar_at_x( x_end , .5 , dy , &len , l , i_bar_last[i-1] );

         i_bar_sum = eh_dbl_array_sum( i_bar_at_x , len );
         i_bar_lost += i_bar_sum;

         eh_free( i_bar_at_x );
      }
      while ( i_bar_sum>.01 );

   }

   return i_bar;
}
*/

double*
plume_i_bar_at_x(double x, double dx, double dy, gint* n_y, double l,
    double* i_bar_last)
{
    double* i_bar = NULL;

    {
        gssize j;
        double* u     = plume_u_bar_at_x(x, dy, n_y);
        double* v     = plume_v_bar_at_x(x, dy, n_y);
        double* k     = plume_k_bar_at_x(x, dy, n_y);
        double* dk_dy = plume_k_bar_dy_at_x(x, dy, n_y);
        double* sub   = eh_new(double, *n_y);
        double* d     = eh_new(double, *n_y);
        double* sup   = eh_new(double, *n_y);
        double* b     = eh_new(double, *n_y);

        i_bar = eh_new(double, *n_y);

        if (x < PLUME_XA) {
            double y_plug = plume_plug_width(x);
            gssize len = y_plug / dy;

            //         if ( len>=(*n_y) )
            //            len = *n_y-1;

            for (j = 0 ; j < len ; j++) {
                i_bar[j] = exp(-l * x);
            }

            if (len < (*n_y - 2) && *n_y > len) {
                for (; j < (*n_y) - 1 ; j++) {
                    sub[j] = - (v[j] - dk_dy[j]) / (2 * dy) - k[j] / (dy * dy);
                    sup[j] = (v[j] - dk_dy[j]) / (2 * dy) - k[j] / (dy * dy);
                    d[j]   =  u[j] / dx + l + 2 * k[j] / (dy * dy);
                    b[j]   = i_bar_last[j] * u[j] / dx;
                }

                // BC at plug boundary.
                b[len] = i_bar_last[len] * u[len] / dx
                    - i_bar[len - 1] * (-(v[len - 1] - dk_dy[len - 1]) / (2 * dy) - k[len - 1] / (dy * dy));

                // BC at plume edge.
                j = *n_y - 2;
                sup[j] = 0.;

                tridiag(sub + len, d + len, sup + len, b + len, i_bar + len, *n_y - 1 - len);
            }
        } else {

            for (j = 1 ; j < (*n_y) - 1 ; j++) {
                sub[j] = - (v[j] - dk_dy[j]) / (2 * dy) - k[j] / (dy * dy);
                sup[j] = (v[j] - dk_dy[j]) / (2 * dy) - k[j] / (dy * dy);
                d[j]   =  u[j] / dx + l + 2 * k[j] / (dy * dy);
                b[j]   = i_bar_last[j] * u[j] / dx;
            }

            // boundary condition at j=0.  Iy = 0 ( I_1 = I_-1 )
            // u_1     = u_-1
            // v_1     = - v_-1
            // k_1     = k_-1
            // dk_dy_1 = -dk_dy_-1
            d[0]   = u[0] / dx + l + 2 * k[0] / (dy * dy);
            sup[0] = (v[1] - dk_dy[1]) / dy - 2.*k[1] / (dy * dy);
            b[0]   = i_bar_last[0] * u[0] / dx;

            // boundary condition at j=N.  I(y=y_c) = 0
            j = *n_y - 1;
            sub[j] = 0.;
            d[j]   = 1.;
            b[j]   = i_bar_last[j] * u[j] / dx;

            j = *n_y - 2;
            sup[j] = 0.;

            tridiag(sub, d, sup, b, i_bar, *n_y - 1);
        }

        //      g_memmove( i_bar , dk_dy , sizeof(double)*(*n_y) );

        eh_free(b);
        eh_free(sup);
        eh_free(d);
        eh_free(sub);
        eh_free(dk_dy);
        eh_free(k);
        eh_free(v);
        eh_free(u);
    }

    return i_bar;
}

double
plume_half_width(double x)
{
    double w;

    eh_require(x >= 0);

    if (x < PLUME_XA) {
        w = plume_establishment_width(x);
    } else {
        w = plume_established_width(x);
    }

    return w;
}

double
plume_established_width(double x)
{
    double w = 0;

    eh_return_val_if_fail(x >= PLUME_XA, 0);

    w = .25 * x;
    //   return plume_establishment_width(x);
    //   w = .25*x + .5;
    return w;
}

double
plume_establishment_width(double x)
{
    double w = 0;

    eh_return_val_if_fail(x >= 0, 0);
    eh_return_val_if_fail(x <= PLUME_XA, 0);

    w = .5 + (.25 - .5 / PLUME_XA) * x;

    //   w = .25*x + .5;
    return w;
}

double
plume_plug_width(double x)
{
    double w = 0;

    eh_return_val_if_fail(x >= 0, 0);
    eh_return_val_if_fail(x <= PLUME_XA, 0);

    w = .5 - .5 / PLUME_XA * x;

    return w;
}

Sed_cell_grid
plume_width_averaged_deposit_num(Sed_cell_grid g, Sed_hydro r, Sed_sediment s,
    double dy)
{
    double* x;

    eh_return_val_if_fail(g, NULL);
    eh_return_val_if_fail(r, NULL);

    // Set up non-dimensional distances
    {
        gssize len = eh_grid_n_y(g);

        x = (double*)g_memdup(eh_grid_y(g), sizeof(double) * len);

        eh_dbl_array_mult(x, len, 1. / sed_hydro_width(r));
    }

    // Integrate the deposit thickness over the grid
    {
        gssize i, n;
        double i_0, rho, l;
        gssize n_grains = sed_sediment_n_types(s);
        gssize len      = eh_grid_n_y(g);
        double* t       = eh_new0(double, n_grains);
        double* dep     = eh_new(double, len);
        double dt       = sed_hydro_duration(r);

        for (n = 1 ; n < n_grains ; n++) {
            l   = plume_non_dim_lambda(sed_type_lambda(sed_sediment_type(s, n)), r);
            rho = sed_type_rho_sat(sed_sediment_type(s, n));
            i_0 = sed_hydro_nth_concentration(r, n - 1) * sed_hydro_depth(r);

            plume_width_averaged_deposit_nd_num(dep, x, len, l);

            for (i = 0 ; i < len - 1 ; i++) {
                //            t[n] = dep[i] * i_0 * pow(sed_hydro_width(r),2) / (dy*sed_hydro_width(r)*(x[i+1]-x[i])*rho)*sed_type_lambda( sed_sediment_type(s,n) );
                t[n] = dep[i] * i_0 / (rho * dy) * sed_type_lambda(sed_sediment_type(s,
                            n)) * sed_hydro_width(r) * dt;
                //            t[n] = dep[i] * i_0 / rho * dt / dy;
                //            t[n] = dep[i] / pow(sed_hydro_width(r),2) / rho * dt;
                sed_cell_add_amount(sed_cell_grid_val(g, 0, i), t);
            }

            t[n] = 0;
        }

        eh_free(t);
        eh_free(dep);
    }

    eh_free(x);

    return g;
}

double*
plume_width_averaged_deposit_nd_num(double* dep, double* x, gssize len, double l)
{
    eh_return_val_if_fail(x, NULL);
    eh_return_val_if_fail(len > 0, NULL);
    eh_return_val_if_fail(l > 0, NULL);

    if (!dep) {
        dep = eh_new(double, len);
    }

    {
        gssize i, j;
        gssize n_y;
        double** i_bar = plume_i_bar(x, len, l, &n_y, .01);

        for (i = 0 ; i < len - 1 ; i++) {
            dep[i] = i_bar[i][0];

            for (j = 1 ; j < n_y ; j++) {
                dep[i] += 2.*i_bar[i][j];
            }

            //         dep[i] *= .04*(x[i+1]-x[i])*2.;
            dep[i] *= .01;
        }

        eh_free(i_bar[0]);
        eh_free(i_bar);
    }

    return dep;
}

/*
Eh_dbl_grid plume_u_bar_grid( gssize n_x , gssize n_y , double dx , double dy )
{
   Eh_dbl_grid u = eh_grid_new_uniform( double , n_x , n_y , dx , dy );

   for ( i=0 ; i<n_x ; i++ )
      for ( j=0 ; j<n_y ; j++ )
         plume_u_bar( eh_grid_x(u)[i] , eh_grid_y(u)[j] );

   return u;
}

double plume_residual( double** inv , gssize n_x , gssize n_y , double l , double dx , double dy )
{
    Eh_dbl_grid r = eh_dbl_grid_new( n_x , n_y );

   {
      gssize i, j;
      Eh_dbl_grid u   = plume_u_bar_grid   ( n_x , n_y , dx , dy );
      Eh_dbl_grid v   = plume_v_bar_grid   ( n_x , n_y , dx , dy );
      Eh_dbl_grid k   = plume_k_bar_grid   ( n_x , n_y , dx , dy );
      Eh_dbl_grid k_y = plume_k_bar_dy_grid( n_x , n_y , dx , dy );
      Eh_dbl_grid inv_x  = eh_dbl_grid_diff( inv , 1 , 2 );
      Eh_dbl_grid inv_y  = eh_dbl_grid_diff( inv , 1 , 1 );
      Eh_dbl_grid inv_yy = eh_dbl_grid_diff( inv , 2 , 1 );


      for ( i=0 ; i<n_x ; i++ )
         for ( j=0 ; j<n_x ; j++ )
            r[i][j] = u[i][j]*inv_x[i][j]
                    + v[i][j]*inv_y[i][j]
                    + k*inv[i][j]
                    - k[i][j]*inv_yy[i][j]
                    - k_y[i][j]*inv_y[i][j];

      eh_grid_destroy( inv_x  );
      eh_grid_destroy( inv_y  );
      eh_grid_destroy( inv_yy );
      eh_grid_destroy( u   );
      eh_grid_destroy( u   );
      eh_grid_destroy( v   );
      eh_grid_destroy( k   );
      eh_grid_destroy( k_y );
   }

}
*/

