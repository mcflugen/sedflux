#include <glib.h>

#include "utils/utils.h"

#include "sed_sediment.h"
#include "sed_wave.h"

/** A structure to describe an ocean wave
*/
CLASS(Sed_wave)
{
    double h; ///< Wave height
    double k; ///< Wavenumber
    double w; ///< Wave frequency
};

/** A structure to describe an ocean storm
*/
CLASS(Sed_ocean_storm)
{
    Sed_wave w; ///< Wave characteristic of the storm
    double val; ///< A scalar to describe the strength of the storm
    double dt;  ///< The length of the storm
    gssize ind; ///< An index to indicate when the storm occured
};

Sed_wave
sed_wave_new(double h, double k, double w)
{
    Sed_wave new_wave = NULL;

    if (h >= 0 && k >= 0 && w >= 0) {
        NEW_OBJECT(Sed_wave, new_wave);

        new_wave->h = h;
        new_wave->k = k;
        new_wave->w = w;
    }

    return new_wave;
}

Sed_wave
sed_wave_copy(Sed_wave dest, Sed_wave src)
{
    eh_require(src);

    if (src) {
        if (!dest) {
            dest = sed_wave_new(src->h, src->k, src->w);
        } else {
            dest->h = src->h;
            dest->w = src->w;
            dest->k = src->k;
        }
    } else {
        dest = NULL;
    }

    return dest;
}

Sed_wave
sed_wave_dup(Sed_wave src)
{
    return sed_wave_copy(NULL, src);
}

gboolean
sed_wave_is_same(Sed_wave w_1, Sed_wave w_2)
{
    gboolean same = FALSE;

    eh_require(w_1);
    eh_require(w_2);

    if (w_1 && w_2) {
        if (w_1 != w_2) {
            same =  fabs(w_1->h - w_2->h) < 1e-12
                && fabs(w_1->w - w_2->w) < 1e-12
                && fabs(w_1->k - w_2->k) < 1e-12;
        } else {
            same = TRUE;
        }
    }

    return same;
}

Sed_wave
sed_wave_destroy(Sed_wave w)
{
    if (w) {
        eh_free(w);
    }

    return NULL;
}

double
sed_wave_height(Sed_wave w)
{
    return w->h;
}

double
sed_wave_number(Sed_wave w)
{
    return w->k;
}

double
sed_wave_length(Sed_wave w)
{
    return 2 * G_PI / w->k;
}

double
sed_wave_frequency(Sed_wave w)
{
    return w->w;
}

double
sed_wave_period(Sed_wave w)
{
    return 2 * G_PI / w->w;
}

double
sed_wave_phase_velocity(Sed_wave w)
{
    return w->w / w->k;
}

gboolean
sed_wave_is_bad(Sed_wave w)
{
    return eh_isnan(sed_wave_height(w)
            * sed_wave_frequency(w)
            * sed_wave_number(w));
}

Sed_wave
sed_gravity_wave_set_frequency(Sed_wave a, double w, double h)
{
    eh_require(h > 0) {
        a->w = w;
        a->k = sed_dispersion_relation_wave_number(h, w);
    }
    return a;
}

Sed_wave
sed_gravity_wave_set_number(Sed_wave w, double k, double h)
{
    eh_require(h > 0) {
        w->k = k;
        w->w = sed_dispersion_relation_frequency(h, k);
    }
    return w;
}

Sed_wave
sed_gravity_wave_set_height(Sed_wave w, Sed_wave w_infinity, double h)
{
    double n = .5 * (1 + 2.*w->k * h / sinh(2 * w->k * h));
    double c          = sed_wave_phase_velocity(w);
    double c_infinity = sed_wave_phase_velocity(w_infinity);

    w->h = w_infinity->h * sqrt(1. / (2 * n) * c_infinity / c);

    return w;
}

Sed_wave
sed_gravity_wave_new(Sed_wave w_infinity, double h, Sed_wave new_wave)
{
    if (!new_wave) {
        new_wave = sed_wave_new(0, 0, 0);
    }

    // Set the frequency (and also the wavenumber from the dispersion
    // relation), and height of the new wave at the specified water depth.
    sed_gravity_wave_set_frequency(new_wave, w_infinity->w, h);
    sed_gravity_wave_set_height(new_wave, w_infinity, h);

    return new_wave;
}

gboolean
sed_wave_is_breaking(Sed_wave w, double h)
{
    return w->h / sed_wave_length(w) >= 1. / 7.;
}

double
sed_gravity_wave_deep_water_height(Sed_wave w)
{
    double kh2;
    double h;
    double x = w->w * w->w / (sed_gravity() * w->k);

    if (fabs(x - 1.) < 1e-5) {
        return w->h;
    } else {
        h = atanh(x) / w->k;
    }

    kh2 = 2 * w->k * h;

    return w->h / sqrt((cosh(kh2) + 1) / (sinh(kh2) + kh2));
}

double
sed_gravity_wave_deep_water_wave_number(Sed_wave w)
{
    return w->w * w->w / sed_gravity();
}

double
sed_wave_break_depth(Sed_wave w)
{
    void sed_wave_break_depth_helper(double k_times_h,
        double * y,
        double * dydx,
        double * data);
    double k_times_h, k, h;
    double h_deep_water;
    double data[2];

    h_deep_water = sed_gravity_wave_deep_water_height(w);

    data[0] = h_deep_water;
    data[1] = w->w;

    k_times_h = rtsafe(&sed_wave_break_depth_helper,
            1e-5,
            100,
            .01,
            data);
    //if ( eh_isnan( k_times_h ) )
    //   eh_warning( "Bad wave in sed_wave_break_depth" );

    k = pow(w->w, 2.) / (sed_gravity() * tanh(k_times_h));

    h = k_times_h / k;

    return h;
}

void
sed_wave_break_depth_helper(double k_times_h,
    double* y,
    double* dydx,
    double* data)
{
    double h   = data[0];
    double w   = data[1];
    double g   = sed_gravity();
    double kh  = k_times_h;
    double kh2 = 2 * k_times_h;

    *y = (1. / 7.) * 2.*G_PI * g * tanh(kh) / (h * w * w)
        - sqrt((cosh(kh2) + 1.) / (sinh(kh2) + kh2));

    *dydx = (1. / 7.) * 2 * G_PI * g / (h * w * w) * pow(1. / cosh(kh), 2.)
        -   .5 / sqrt((cosh(kh2) + 1) / (sinh(kh2) + kh2))
        * (2 * (sinh(kh2) + kh2) * sinh(kh2) - (cosh(kh2) + 1) * (2 * cosh(kh2) + 2))
        / pow(sinh(kh2) + kh2, 2);

    return;
}

double
sed_dispersion_relation_frequency(double water_depth, double wave_number)
{
    double w = eh_nan();

    eh_require(water_depth > 0) {
        w = sqrt(sed_gravity() * wave_number * tanh(wave_number * water_depth));
    }

    return w;
}

/** Solve the dispersion relation for wave number

The dispersion relation for gravity waves is

\f[
   \omega^2 = g \kappa \tanh\left( \kappa z \right)
\f]

where \f$ \omega \f$ is wave frequency, \f$ f \f$ is acceleration due
to gravity, \f$ \kappa \f$ is wave number, and \f$ z \f$ is water depth.

\param water_depth Water depth in meters
\param frequency   Wave frequencey in 1/s

\return Wave number in 1/m
*/
double
sed_dispersion_relation_wave_number(double water_depth,
    double frequency)
{
    void sed_dispersion_relation_wave_number_helper(double k,
        double * y,
        double * dydx,
        double * data);
    double wave_number = eh_nan();

    eh_require(water_depth > 0) {
        double data[2];

        data[0] = water_depth;
        data[1] = frequency;

        wave_number = rtsafe(&sed_dispersion_relation_wave_number_helper,
                0,
                100,
                .01,
                data);
    }

    return wave_number;
}

void
sed_dispersion_relation_wave_number_helper(double k,
    double* y,
    double* dydx,
    double* data)
{
    double g = sed_gravity();
    double h = data[0]; // water depth
    double w = data[1]; // frequency

    *y    = g * k * tanh(k * h) - w * w;
    *dydx = g * (k * h * pow(1. / cosh(k * h), 2.) + tanh(k * h));
}

Sed_ocean_storm
sed_ocean_storm_new(void)
{
    Sed_ocean_storm s;

    NEW_OBJECT(Sed_ocean_storm, s);

    s->w   = sed_wave_new(0, 0, 0);
    s->val = 0;
    s->ind = 0;
    s->dt  = 0;

    return s;
}

Sed_ocean_storm
sed_ocean_storm_destroy(Sed_ocean_storm s)
{
    if (s) {
        sed_wave_destroy(s->w);
        eh_free(s);
    }

    return NULL;
}

double
sed_ocean_storm_duration(Sed_ocean_storm s)
{
    return s->dt;
}

gssize
sed_ocean_storm_index(Sed_ocean_storm s)
{
    return s->ind;
}

double
sed_ocean_storm_val(Sed_ocean_storm s)
{
    return s->val;
}

double
sed_ocean_storm_duration_in_seconds(Sed_ocean_storm s)
{
    return s->dt * S_SECONDS_PER_DAY;
}

double
sed_ocean_storm_wave_height(Sed_ocean_storm s)
{
    return sed_wave_height(s->w);
}

double
sed_ocean_storm_wave_number(Sed_ocean_storm s)
{
    return sed_wave_number(s->w);
}

double
sed_ocean_storm_wave_length(Sed_ocean_storm s)
{
    return sed_wave_length(s->w);
}

double
sed_ocean_storm_wave_freq(Sed_ocean_storm s)
{
    return sed_wave_frequency(s->w);
}

double
sed_ocean_storm_wave_period(Sed_ocean_storm s)
{
    return sed_wave_period(s->w);
}

double
sed_ocean_storm_phase_velocity(Sed_ocean_storm s)
{
    return sed_wave_phase_velocity(s->w);
}

Sed_ocean_storm
sed_ocean_storm_set_wave(Sed_ocean_storm s, Sed_wave w)
{
    eh_require(w);

    if (s) {
        sed_wave_copy(s->w, w);
    }

    return s;
}

Sed_ocean_storm
sed_ocean_storm_set_index(Sed_ocean_storm s, gssize ind)
{
    if (s) {
        s->ind = ind;
    }

    return s;
}

Sed_ocean_storm
sed_ocean_storm_set_duration(Sed_ocean_storm s, double dt_in_days)
{
    if (s) {
        s->dt = dt_in_days;
    }

    return s;
}

Sed_ocean_storm
sed_ocean_storm_set_val(Sed_ocean_storm s, double val)
{
    if (s) {
        s->val = val;
    }

    return s;
}

gssize
sed_ocean_storm_fprint(FILE* fp, Sed_ocean_storm s)
{
    gssize n = 0;

    eh_require(fp);
    eh_require(s);

    if (s) {
        n += fprintf(fp, "Time index      : %d", (gint)s->ind);
        n += fprintf(fp, "Value           : %f", s->val);
        n += fprintf(fp, "Duration (days) : %f", s->dt);
        n += fprintf(fp, "Wave height (m) : %f", sed_wave_height(s->w));
        n += fprintf(fp, "Wave length (m) : %f", sed_wave_length(s->w));
        n += fprintf(fp, "Wave period (m) : %f", sed_wave_period(s->w));
    }

    return n;
}

