#include <stdio.h>
#include <glib.h>
#include <sed/sed_diag.h>

typedef enum {
    SED_DIAG_TARGET_CUBE = 0,
    SED_DIAG_TARGET_COLUMN,
    SED_DIAG_TARGET_CELL,
    SED_DIAG_TARGET_UNKNOWN,
} Sed_diag_type;

CLASS(Sed_diag)
{
    Sed_cube   cube;
    Sed_column col;
    Sed_cell   cell;

    double     start;
    double     stop;

    Sed_diag_type type;
};

Sed_diag
sed_diag_new(void)
{
    Sed_diag d = NULL;

    NEW_OBJECT(Sed_diag, d);

    d->cube  = NULL;
    d->col   = NULL;
    d->cell  = NULL;

    d->start = 0;
    d->stop  = 0;

    d->type  = SED_DIAG_TARGET_UNKNOWN;

    return d;
}

Sed_diag
sed_diag_destroy(Sed_diag d)
{
    if (d) {
        eh_free(d);
    }

    return NULL;
}

Sed_diag
sed_diag_new_target_cube(Sed_cube   c)
{
    Sed_diag d = NULL;

    if (c) {
        d       = sed_diag_new();
        d->cube = c;
        d->type = SED_DIAG_TARGET_CUBE;
    }

    return d;
}

Sed_diag
sed_diag_new_target_column(Sed_column c)
{
    Sed_diag d = NULL;

    if (c) {
        d       = sed_diag_new();
        d->col  = c;
        d->type = SED_DIAG_TARGET_COLUMN;
    }

    return d;
}

Sed_diag
sed_diag_new_target_cell(Sed_cell   c)
{
    Sed_diag d = NULL;

    if (c) {
        d       = sed_diag_new();
        d->cell = c;
        d->type = SED_DIAG_TARGET_CELL;
    }

    return d;
}

double
_sed_diag_mass(Sed_diag d)
{
    double m = 0;

    switch (d->type) {
        case SED_DIAG_TARGET_CUBE  :
            m = sed_cube_sediment_mass(d->cube);
            break;

        case SED_DIAG_TARGET_COLUMN:
            m = sed_column_sediment_mass(d->col);
            break;

        case SED_DIAG_TARGET_CELL  :
            m = sed_cell_sediment_mass(d->cell);
            break;

        default:
            eh_require_not_reached();
    }

    return m;
}

double
sed_diag_start(Sed_diag d)
{
    double m = 0;

    if (d) {
        d->start = _sed_diag_mass(d);
    }

    return m;
}

double
sed_diag_stop(Sed_diag d)
{
    double m = 0;

    if (d) {
        d->stop = _sed_diag_mass(d);
    }

    return m;
}

double
sed_diag_elapsed(Sed_diag d)
{
    double m = 0;

    if (d) {
        m = _sed_diag_mass(d) - d->start;
    }

    return m;
}

double
sed_diag_continue(Sed_diag d)
{
    double m = 0;

    if (d) {
        d->stop  = 0;

        m = d->start;
    }

    return m;
}

double
sed_diag_reset(Sed_diag d)
{
    double m = 0;

    if (d) {
        d->start = _sed_diag_mass(d);
        d->stop  = 0;

        m = d->start;
    }

    return m;
}

static const gchar* target_s[] = {
    "Sed_cube",
    "Sed_column",
    "Sed_cell",
    "Unknown"
};

const gchar*
_sed_diag_target_s(Sed_diag d)
{
    return target_s[d->type];
}

gint
sed_diag_fprint(FILE* fp, Sed_diag d)
{
    gint n = 0;

    if (d) {
        fprintf(fp, "--- Start: sedflux diagnostics ---\n");
        fprintf(fp, "Target type     : %s\n", _sed_diag_target_s(d));
        fprintf(fp, "Starting mass   : %g\n", d->start);
        fprintf(fp, "Stopping mass   : %g\n", d->stop);
        fprintf(fp, "--- End: sedflux diagnostics ---\n");
    }

    return n;
}

