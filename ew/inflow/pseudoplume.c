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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
//#include <libgen.h>
#include "inflow.h"
#include <sed/sed_sedflux.h>
#include <utils/utils.h>

/*** Self Documentation ***/
static const char* help_msg[] = {
    "                                                                             ",
    " pseudoplume [options] [parameters]  [filein]                                ",
    "  run a pseudo-plume.                                                        ",
    "                                                                             ",
    " Options                                                                     ",
    "  -o outfile  - send output to outfile instead of stdout. [stdout]           ",
    "  -v          - be verbose. [off]                                            ",
    "  -h          - print this help message.                                     ",
    "                                                                             ",
    " Parameters                                                                  ",
    "  -pday=value - set the length of a day to be value fraction of 24 hrs. [1]  ",
    "  -pb0=value  - width of the river mouth (m). [1]                            ",
    "  -pu0=value  - velocity at the river mouth (m/s). [1]                       ",
    "  -pI0=value  - sediment inventory at the river mouth (m/s). [1]             ",
    "  -plambda=value  - removal rate for the sediment (1/day). [10]              ",
    "                                                                             ",
    "  -fin        - input bathymetry file. [stdin]                               ",
    "  -fout       - output bathymetry file. [stdout]                             ",
    NULL
};

#ifndef M_PI
    #define M_PI        3.14159265358979323846
#endif

#define DEFAULT_DAY_LENGTH           1
#define DEFAULT_VERBOSE              0
#define DEFAULT_U0                   (1.)
#define DEFAULT_B0                   (1.)
#define DEFAULT_I0                   (1.)
#define DEFAULT_LAMBDA               (10.)

double*
pseudoplume(double* x, int len_x, double lambda, double* I);

int
main(int argc, char* argv[])
{
    FILE* fp_out;
    Eh_args* args;
    char* infile, *outfile;
    int i,  n_nodes;
    gboolean verbose;
    double day, u0, b0, inv0, lambda;
    double* x, *y, *z;
    double* I;

    args = eh_opts_init(argc, argv);

    if (eh_check_opts(args, NULL, NULL, help_msg) != 0) {
        eh_exit(-1);
    }

    verbose = eh_get_opt_bool(args, "v", FALSE);
    day     = eh_get_opt_dbl(args, "day", DEFAULT_DAY_LENGTH);
    b0      = eh_get_opt_dbl(args, "b0", DEFAULT_B0);
    u0      = eh_get_opt_dbl(args, "u0", DEFAULT_U0);
    inv0    = eh_get_opt_dbl(args, "I0", DEFAULT_I0);
    lambda  = eh_get_opt_dbl(args, "lambda", DEFAULT_LAMBDA);
    infile  = eh_get_opt_str(args, "in", NULL);
    outfile = eh_get_opt_str(args, "out", NULL);

    lambda /= 86400.;

    if (verbose) {
        fprintf(stderr, "pseudoplume : river width (m)      : %f\n", b0);
        fprintf(stderr, "pseudoplume : river velocity (m/s) : %f\n", u0);
        fprintf(stderr, "pseudoplume : removal rate (1/s)   : %f\n", lambda);
    }

    // Read input bathymetry.
    {
        Eh_data_record* all_records = eh_data_record_scan_file(infile, ",", EH_FAST_DIM_COL,
                FALSE, NULL);

        x = eh_data_record_dup_row(all_records[0], 0);
        y = eh_data_record_dup_row(all_records[0], 1);
        z = eh_data_record_dup_row(all_records[0], 2);

        for (i = 0 ; all_records[i] ; i++) {
            eh_data_record_destroy(all_records[i]);
        }

        eh_free(all_records);
    }

    // Allocate space for the plume deposit.
    I = eh_new(double, n_nodes);

    // Non-dimesionalize x, and lambda.
    for (i = 0; i < n_nodes; i++) {
        x[i] /= b0;
    }

    lambda *= (b0 / u0);

    // Run the pseudoplume.
    pseudoplume(x, n_nodes, lambda, I);

    // Redimensionalize the output.
    for (i = 0; i < n_nodes; i++) {
        x[i] *= b0;
        I[i] *= inv0;
    }

    // Add the pseudoplume deposit to the bathymetry.
    for (i = 0; i < n_nodes; i++) {
        y[i] += I[i];
    }

    // Write out the new bathymetry.
    fp_out = eh_open_file(outfile, "w");

    for (i = 0; i < n_nodes; i++) {
        fprintf(fp_out, "x, y, w : %f, %f, %f\n", x[i], I[i], z[i]);
    }

    fclose(fp_out);

    eh_free(I);
    eh_free(x);
    eh_free(y);
    eh_free(z);

    return 0;
}

/////////////////
//
// x      - non-dimensional distances from the river mouth.
// len_x  - length of vector x.
// lambda - non-dimensional removal rate.
// I      - resulting non-dimentional inventory.
//
// The variables are non-dimensionalized as follows:
//   x = x'/b0
//   lambda = lambda' * b0 / u0
//
// where,
//   x' = dimesional distance
//   lambda' = dimensional removal rate
//   b0 = dimensional river mouth width
//   u0 = dimensional river mouth velocity
//
/////////////////

#define X_A (5.176)
#define PSEUDOPLUME_A (0.126)
#define PSEUDOPLUME_B (1.)

double*
pseudoplume(double* x, int len_x, double lambda, double* I)
{
    int i;
    double x_a = X_A;
    double a = PSEUDOPLUME_A;
    double b = PSEUDOPLUME_B;

    for (i = 0; i < len_x; i++) {
        if (x[i] < x_a) {
            I[i] = exp(-lambda * x[i]);
        } else {
            I[i] = exp(-lambda * x_a / 3.*(1 + 2 * pow(x[i] / x_a,
                            1.5))) * (1. / (1 + a * pow(x[i] - x_a, b)));
        }
    }

    return I;
}

