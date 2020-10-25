/*
 *  HydroCommandLine.c
 *
 *
 *  Author:    A.J. Kettner (April 2003)
 *
 *  HydroCommandline handles the commandline imput to set web variables
 *  and the output directory structure.
 *
 *
 * Variable     Def.Location        Type        Units   Usage
 * --------     ------------        ----        -----   -----
 * input_in     HydroCommandline.c  char        -       string variable to set the input directory
 *
 */

#include <string.h>
#include "hydroclimate.h"
#include "hydroinout.h"
#include "hydroparams.h"
#include "hydroalloc_mem.h"
#define MAXLENGTH (80)
#define TEST1 (2)
#define TEST5 (5)

/*-----------------------------
 *  Start of HydroCommandLine
 *-----------------------------*/
int
hydrocommandline(int* argc, char** argv)
{

    /*-------------------
     *  Local Variables
     *-------------------*/
    int err, jj;
    char input_in[300];

    /*------------------------
     *  Initialize Variables
     *------------------------*/
    err = 0;

    for (jj = 0; jj < *argc; jj++) {
        strcpy(commandlinearg[jj], argv[jj]);
    }

    if (*argc == 1) {
        /*---------------------------------------------------
         *  argc = 1 creats standard output filenames named
         *  HYDRO.*
         *---------------------------------------------------*/
        strcpy(commandlinearg[1], DUMMY);
        fprintf(stderr, "\n  Hydrotrend started without project title. Output will \n");
        fprintf(stderr, "  be written to standard output files, named %s.\n",
            commandlinearg[1]);
        fprintf(stderr, "  Old files will be overwritten!!\n\n");
    }

    if (*argc == 2)

        /*------------------------------------------------
         *  argc = 2 makes it possible to make different
         *  filenames per each run.
         *------------------------------------------------*/
        for (jj = 0; jj < MAXLENGTH; jj++) {
            if (commandlinearg[1][jj] == '%' || commandlinearg[1][jj] == '*'
                || commandlinearg[1][jj] == '#'
                || commandlinearg[1][jj] == '@' || commandlinearg[1][jj] == '-'
                || commandlinearg[1][jj] == '^'
                || commandlinearg[1][jj] == '"' || commandlinearg[1][jj] == '?'
                || commandlinearg[1][jj] == '!'
                || commandlinearg[1][jj] == '.' || commandlinearg[1][jj] == ','
                || commandlinearg[1][jj] == '$') {
                fprintf(stderr, "  HydroTrend ERROR: Incorrect command line \n");
                fprintf(stderr, "    You can use only characters and numbers to name the project. \n");
                fprintf(stderr, "Don't use %c.\n", commandlinearg[1][jj]);
                err++;
            }

            commandlinearg[1][jj] = toupper(commandlinearg[1][jj]);
        }

    if (*argc == 3) {
        /*------------------------------------------------
         *  argc = 3 is reserved for the web application,
         *  so only use a third argument for the web
         *------------------------------------------------*/
        for (jj = 0; jj < MAXLENGTH; jj++)
            if (commandlinearg[2][jj] != '0' && commandlinearg[2][jj] != '1'
                && commandlinearg[2][jj] != '2'
                && commandlinearg[2][jj] != '3' && commandlinearg[2][jj] != '4'
                && commandlinearg[2][jj] != '5'
                && commandlinearg[2][jj] != '6' && commandlinearg[2][jj] != '7'
                && commandlinearg[2][jj] != '8'
                && commandlinearg[2][jj] != '9') {
                commandlinearg[2][jj]  = '/';
                commandlinearg[2][jj + 1] = '\0';
                jj = MAXLENGTH;
            }

        webflag = 1;
        strcpy(input_in,   INPUTSTRING);
        strcat(input_in,   commandlinearg[2]);
        strcat(input_in,   commandlinearg[1]);
        strcat(ffnameinput, input_in);
        strcpy(ffnamehyps, input_in);
        strcpy(ffnameinputgw_r, input_in);
        strcat(ffnameinputgw_r, fnameclimateext);
        strcat(ffnameinput, fnameinputext);
        strcat(ffnamehyps, fnamehypsext);
    }

    if (*argc > 3) {
        /*-----------------------------------------------
         *  argc > 3 does not exist. It will handle the
         *  output as if argc = 1.
         *-----------------------------------------------*/
        strcpy(commandlinearg[1], DUMMY);
        fprintf(stderr, "  HydroTrend ERROR: Incorrect command line \n");
        fprintf(stderr, "    argc should equal 1 or 2 \n");
        fprintf(stderr, "    argc = %d \n", *argc);
        fprintf(stderr, "    HydroTrend does not use more than 2 command line arguments. \n");
        fprintf(stderr, "    Ignoring all arguments. \n");
        fprintf(stderr, "    Output will be written to standard output files, named %s.\n",
            commandlinearg[1]);
    }

    return (err);
} /* HydroCommandLine.c */
