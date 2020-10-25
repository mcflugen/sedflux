/*
 *  HydroShoulder.c

 *  Calculates the flood wave shoulder parameters.
 *  The shoulder's simulate the dispersion of a flood
 *  wave as it propagates down channel.
 *
 *  Author:    M. Nicholson in river.f
 *  Modified:  M.D. Morehead for HydroTrend.c
 *  Original:  Jan 1999
 *
 * Variable     Def.Location    Type    Units   Usage
 * --------     ------------    ----    -----   -----
 * err          various         int     -       errorflag, halts program
 * daysum       HydroShoulder.c double  -       used for shoulder event calculation
 * diff12       HydroShoulder.c double  -       used for shoulder event calculation
 * dumdbl       various         double  -       temporary double
 * jj           various         int     -       temporary loop counter
 * dayoffset    HydroShoulder.c double  -       used for shoulder event calculation
 * leftpercent  HydroShoulder.c double  -       used for shoulder event calculation
 * maxpercent   HydroShoulder.c double  -       used for shoulder event calculation
 *
 */

#include <math.h>

#include "hydroclimate.h"
#include "hydroparams.h"

#ifdef DBG
    #include "hydroinout.h"
#endif

/*--------------------------
 *  Start of HydroShoulder
 *--------------------------*/
int
hydroshoulder()
{

    /*-------------------
     *  Local Variables
     *-------------------*/
    int err, jj;
    double  diff12, daysum, dumdbl;
    double  dayoffset, leftpercent, maxpercent;

    err = 0;

    /*--------------------------------------------------------------------
     *  Set the Shoulder parameters
     *
     *  dayoffset is the minimum number of days to spread the event over
     *  leftpercent is a scaled version of how much goes to the day
     *    prior to the main event (if dayoffset = 3)
     *  maxpercent is a scaled values of how much goes to the main event
     *
     *  should.m is a matlab script that can be used to investigate how
     *  this function works.
     *
     *  NOTE: some values will cause negative flows!!!
     *
     *  Original values from Murray Nicholson are:
     *    dayoffset = 3.0;
     *    leftpercent   = 0.4;
     *    maxpercent    = 1.0;
     *--------------------------------------------------------------------*/
    dayoffset   = 3.0;
    leftpercent = 0.65;
    maxpercent  = 0.7;

    /*-------------------------------------------------------------
     *  Calculate the event shoulder numbers
     *  These depend on variables that only change with the epoch
     *  Might move this into a subroutine
     *-------------------------------------------------------------*/
    for (jj = 0; jj < maxshoulder; jj++) {
        shoulderright[jj] = 0.0;
    }

    /*----------------------------------------------------------------
     *  Compute the number of days required for a flow to travel the
     *  length of the basin, set 3 as the minimum
     *----------------------------------------------------------------*/
    shouldern = mx((int)(basinlength[ep] / (avgvel[ep] * dTOs)), 3);

    /*------------------------------------------------------------
     *  account for lag due the % of the drainage basin as lakes
     *    if( Rvol > 0.0 && < 0.5 ) shouldern += 1
     *    if( Rvol >= 0.5 ) shouldern += 2
     *------------------------------------------------------------*/
    if (Rvol[ep] > 0.0 && Rvol[ep] < 0.5) {
        shouldern += (int)floor(.4 * 3.3333);
    }

    if (Rvol[ep] >= 0.5) {
        shouldern += (int)floor(.8 * 3.3333);
    }

    /*-------------------------------------------------
     *  assign percentage values to modulate the flow
     *-------------------------------------------------*/
    shoulderleft = sin(PI / ((double)shouldern + dayoffset)) * leftpercent;
    shouldermain = sin(PI / ((double)shouldern + dayoffset)) * maxpercent;
    diff12 = 1.00 - (shouldermain + shoulderleft);

    daysum = 0;

    for (jj = 1; jj < shouldern - 1; jj++) {
        daysum += jj;
    }

    for (jj = 0; jj < shouldern - 2; jj++) {
        shoulderright[jj] = diff12 * (double)((shouldern - 2 - jj) / daysum);
    }

#ifdef DBG
    fprintf(fidlog, "\t HydroShoulder: \n");
    fprintf(fidlog, "\t\t dayoffset     = %f \n", dayoffset);
    fprintf(fidlog, "\t\t leftpercent   = %f \n", leftpercent);
    fprintf(fidlog, "\t\t maxpercent    = %f \n", maxpercent);
    fprintf(fidlog, "\t\t shouldern     = %d \n", shouldern);
    fprintf(fidlog, "\t\t shoulderleft  = %f \n", shoulderleft);
    fprintf(fidlog, "\t\t shouldermain  = %f \n", shouldermain);

    for (jj = 0; jj < shouldern - 2; jj++) {
        fprintf(fidlog, "\t\t shoulderright[jj]  = %f \n", shoulderright[jj]);
    }

#endif

    /*-----------------------------------------------------
     *  Sum the shoulder values to insure they add to one
     *-----------------------------------------------------*/
    dumdbl = 0.0;

    for (jj = 0; jj < shouldern - 2; jj++) {
        dumdbl += shoulderright[jj];
    }

    dumdbl += shoulderleft + shouldermain;

    if (fabs(dumdbl - 1.0) > 0.000001) {
        fprintf(stderr, "ERROR in HydroShoulder.c: \n");
        fprintf(stderr, "   The shoulder events do not sum to 1.0 \n");
        fprintf(stderr, "   sum(shoulder) = %f \n", dumdbl);
        fprintf(stderr, "   shoulderleft  = %f \n", shoulderleft);
        fprintf(stderr, "   shouldermain  = %f \n", shouldermain);

        for (jj = 0; jj < shouldern - 2; jj++) {
            fprintf(stderr, "   shoulderright[jj] = %f \n", shoulderright[jj]);
        }

        err = 1;
    }

    return (err);
}   /* end of HydroShoulder.c */

