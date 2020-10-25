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
#include <stdlib.h>
#include <string.h>
#include <sed/sed_sedflux.h>

#define VAL_NOT_GIVEN (G_MINDOUBLE)

#define THIS_IS_OBSOLETE

int
main(int argc, char* argv[])
{
#if !defined( THIS_IS_OBSOLETE )

    void doHelp(int);
    FILE* fpin, *fpout;
    char* infile, *outfile;
    int i;
    int rows, cols;
    double old_max, old_min, new_max, new_min;
    Profile_header* header;
    double scale, old_scale, new_scale, new_val;
    unsigned char* data;
    Eh_args* args;

    args = eh_opts_init(argc, argv);

    if (eh_check_opts(args, NULL, NULL, NULL) != 0) {
        eh_exit(-1);
    }

    new_min = eh_get_opt_dbl(args, "low", VAL_NOT_GIVEN);
    new_max = eh_get_opt_dbl(args, "high", VAL_NOT_GIVEN);
    infile  = eh_get_opt_str(args, "in", NULL);
    outfile = eh_get_opt_str(args, "out", NULL);

    if (!infile) {
        fpin = stdin;
    } else if (!(fpin = fopen(infile, "r"))) {
        perror(infile), eh_exit(-1);
    }

    if (!outfile) {
        fpout = stdout;
    } else if (!(fpout = fopen(outfile, "w"))) {
        perror(outfile), eh_exit(-1);
    }

    fprintf(stderr, "Scaling data between %f and %f\n", new_min, new_max);

    /* Read the header */
    header = sed_read_profile_header(fpin);

    rows    = header->n_rows;
    cols    = header->n_cols;
    old_min = header->min_value;
    old_max = header->max_value;

    if (new_min == VAL_NOT_GIVEN) {
        new_min = old_min;
    }

    if (new_max == VAL_NOT_GIVEN) {
        new_max = old_max;
    }

    scale = (old_max - old_min) / (new_max - new_min);
    old_scale = (255 - 2) / (old_max - old_min);
    new_scale = (255 - 2) / (new_max - new_min);

    /* read the data */
    data = g_new(unsigned char, rows * cols);
    fread(data, sizeof(unsigned char), rows * cols, fpin);

    /* scale the data */
    header->min_value = new_min;
    header->max_value = new_max;

    for (i = 0; i < rows * cols; i++) {
        if (!(data[i] == 0 || data[i] == 0xFF)) {
            new_val = (((data[i] - 1) / old_scale + old_min) - new_min) * new_scale + 1;

            if (new_val > 0xFF) {
                new_val = 0xFF;
            }

            data[i] = (unsigned char)new_val;
        }
    }

    /* write the header */
    sed_print_profile_header(fpout, header);

    /* write the data */
    fwrite(data, sizeof(unsigned char), rows * cols, fpout);

    fclose(fpin);
    fclose(fpout);
#endif

    return 0;
}

#undef THIS_IS_OBSOLETE

void
do_help(int verbose)
{

    fprintf(stderr, "Usage: sedrescale [help] [options] [parameters] [filein]\n");

    if (!verbose) {
        return;
    }

    fprintf(stderr, "Options:\n");
    fprintf(stderr, "\t-o FILENAME -- Write ouput to the file, FILENAME. [ stdout ]\n");
    fprintf(stderr, "Parameters:\n");
    fprintf(stderr, "\tlow=      -- Lower limit for scaling. [ 0 ]\n");
    fprintf(stderr, "\thigh=     -- Upper limit for scaling. [ 1 ]\n");
    fprintf(stderr, "Input Files:\n");
    fprintf(stderr, "\tfilein      -- File of sedflux output data. [ stdin ]\n");

    return;

}
