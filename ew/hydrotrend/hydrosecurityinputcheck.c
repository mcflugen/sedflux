/*
 *  HydroSecurityInputCheck.c
 *
 *  Checks the input parameters for illigal characters that
 *  can couse trouble with a web based Hydrotrend model.
 *
 *  Author:   A.J. Kettner  (September 2002)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "hydroinout.h"
#include "hydroparams.h"
#define NROWS (500)
#define NCOLS (130)

/*------------------------------------
 *  Start of HydroSecurityInputCheck
 *------------------------------------*/
int
hydrosecurityinputcheck()
{

    /*-------------------
     *  Local Variables
     *-------------------*/
    int jj, ii, err, verbose, i;
    char x;
    err = 0;
    verbose = 0;
    i = 0;

    /*------------------------
     *  Open the input files
     *------------------------*/
    if ((fidinput = fopen(ffnameinput, "r")) == NULL) {
        fprintf(stderr,
            "  HydroSecurityinputcheck.c ERROR: Unable to open the input file %s \n", ffnameinput);
        fprintf(stderr, "    Make sure the input file name is all in capitals\n");
        fprintf(stderr, "    program aborted \n");
        exit(1);
    }

    if ((fidhyps[i] = fopen(ffnamehyps, "r")) == NULL) {
        fprintf(stderr,
            "  openfiles HydroSecurityinputcheck.c ERROR: Unable to open the hypsometeric integral data file %s \n",
            ffnamehyps);
        fprintf(stderr, "    Make sure the input file name is all in capitals\n");
        fprintf(stderr, "    program aborted \n");
        exit(1);
    }

    if ((fidinputgw_r = fopen(ffnameinputgw_r, "r")) == NULL) {
        fprintf(stderr, "  HydroSecurityinputcheck.c MESSAGE: Unable to open input file %s \n",
            ffnameinputgw_r);
        fprintf(stderr, "    Hydrotrend will generate it's own climate values based on\n");
        fprintf(stderr, "    line 14-25 of the input values in the input file.\n\n");
        raindatafile = 0;
    }

    /*----------------------------------------
     *  Scan every line of the input files
     *  for characters which are not allowed
     *----------------------------------------*/
    for (jj = 0; jj < NROWS; jj++)
        for (ii = 0; ii < NCOLS; ii++) {
            fscanf(fidinput, "%c", &x);

            if (x != 'a' && x != 'b' && x != 'c' && x != 'd' && x != 'e' && x != 'f' && x != 'g'
                && x != 'h'
                && x != 'i' && x != 'j' && x != 'k' && x != 'l' && x != 'm' && x != 'n' && x != 'o'
                && x != 'p'
                && x != 'q' && x != 'r' && x != 's' && x != 't' && x != 'u' && x != 'v' && x != 'w'
                && x != 'x'
                && x != 'y' && x != 'z'
                && x != '.' && x != '/' && x != '(' && x != ')' && x != ':' && x != ';' && x != '-'
                && x != '_'
                && x != '+' && x != '=' && x != ' ' && x != ',' && x != '^' && x != '>' && x != '%'
                && x != '\r' && x != '\t' && x != '\0' && x != '\n'
                && x != 'A' && x != 'B' && x != 'C' && x != 'D' && x != 'E' && x != 'F' && x != 'G'
                && x != 'H'
                && x != 'I' && x != 'J' && x != 'K' && x != 'L' && x != 'M' && x != 'N' && x != 'O'
                && x != 'P'
                && x != 'Q' && x != 'R' && x != 'S' && x != 'T' && x != 'U' && x != 'V' && x != 'W'
                && x != 'X'
                && x != 'Y' && x != 'Z'
                && x != '1' && x != '2' && x != '3' && x != '4' && x != '5' && x != '6'
                && x != '7' && x != '8' && x != '9' && x != '0') {
                chrdump[err] = x;
                err++;
            }
        }

    for (jj = 0; jj < NROWS; jj++)
        for (ii = 0; ii < NCOLS; ii++) {
            fscanf(fidhyps[i], "%c", &x);

            if (x != 'a' && x != 'b' && x != 'c' && x != 'd' && x != 'e' && x != 'f' && x != 'g'
                && x != 'h'
                && x != 'i' && x != 'j' && x != 'k' && x != 'l' && x != 'm' && x != 'n' && x != 'o'
                && x != 'p'
                && x != 'q' && x != 'r' && x != 's' && x != 't' && x != 'u' && x != 'v' && x != 'w'
                && x != 'x'
                && x != 'y' && x != 'z'
                && x != '.' && x != '/' && x != '(' && x != ')' && x != ':' && x != ';' && x != '-'
                && x != '_'
                && x != '+' && x != '=' && x != ' ' && x != ',' && x != '^' && x != '>' && x != '%'
                && x != '\r' && x != '\t' && x != '\0' && x != '\n'
                && x != 'A' && x != 'B' && x != 'C' && x != 'D' && x != 'E' && x != 'F' && x != 'G'
                && x != 'H'
                && x != 'I' && x != 'J' && x != 'K' && x != 'L' && x != 'M' && x != 'N' && x != 'O'
                && x != 'P'
                && x != 'Q' && x != 'R' && x != 'S' && x != 'T' && x != 'U' && x != 'V' && x != 'W'
                && x != 'X'
                && x != 'Y' && x != 'Z'
                && x != '1' && x != '2' && x != '3' && x != '4' && x != '5' && x != '6'
                && x != '7' && x != '8' && x != '9' && x != '0') {
                chrdump[err] = x;
                err++;
            }
        }

    if (raindatafile == 1)
        for (jj = 0; jj < NROWS; jj++)
            for (ii = 0; ii < NCOLS; ii++) {
                fscanf(fidinputgw_r, "%c", &x);

                if (x != 'a' && x != 'b' && x != 'c' && x != 'd' && x != 'e' && x != 'f' && x != 'g'
                    && x != 'h'
                    && x != 'i' && x != 'j' && x != 'k' && x != 'l' && x != 'm' && x != 'n' && x != 'o'
                    && x != 'p'
                    && x != 'q' && x != 'r' && x != 's' && x != 't' && x != 'u' && x != 'v' && x != 'w'
                    && x != 'x'
                    && x != 'y' && x != 'z'
                    && x != '.' && x != '/' && x != '(' && x != ')' && x != ':' && x != ';' && x != '-'
                    && x != '_'
                    && x != '+' && x != '=' && x != ' ' && x != ',' && x != '^' && x != '>' && x != '%'
                    && x != '[' && x != ']'
                    && x != '\r' && x != '\t' && x != '\0' && x != '\n'
                    && x != 'A' && x != 'B' && x != 'C' && x != 'D' && x != 'E' && x != 'F' && x != 'G'
                    && x != 'H'
                    && x != 'I' && x != 'J' && x != 'K' && x != 'L' && x != 'M' && x != 'N' && x != 'O'
                    && x != 'P'
                    && x != 'Q' && x != 'R' && x != 'S' && x != 'T' && x != 'U' && x != 'V' && x != 'W'
                    && x != 'X'
                    && x != 'Y' && x != 'Z'
                    && x != '1' && x != '2' && x != '3' && x != '4' && x != '5' && x != '6'
                    && x != '7' && x != '8' && x != '9' && x != '0') {
                    chrdump[err] = x;
                    err++;
                }
            }

    /*---------------------------
     *  Rewind the file for use
     *---------------------------*/
    rewind(fidinput);
    rewind(fidhyps[i]);

    if (raindatafile == 1) {
        fclose(fidinputgw_r);
    }

    return (err);
}  /* end of HydroSecurityInputCheck */
