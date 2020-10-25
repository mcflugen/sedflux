/*
 *  HydroExpDist.c  Estimates a rainfall distribution:
 *          takes (exp(normal-distribution))^1.35
 *          matches the STD exactly
 *
 * Variable     Def.Location    Type    Units   Usage
 * --------     ------------    ----    -----   -----
 * da[ntot]     HydroExpDist    double  m       double array of precip distribution
 * db[ntot]     HydroExpDist    double  m       double array of precip distribution
 * dc[ntot]     HydroExpDist    double  m       double array of precip distribution
 * dumdbl       HydroExpDist    double  m       temporary double
 * err          various         int     -       error flag, halts program
 * ii           various         int     -       temporary loop counter
 * kk           various         int     -       temporary loop counter
 * mnth         HydroExpDist    int     -       month of the year
 * pvals[31]    HydroExpDist    double  m       daily precipitation array for a month
 * stda         HydroExpDist    double  -       STD of the 'a' distribution values
 * stdb         HydroExpDist    double  -       STD of the 'b' distribution values
 * sumx         HydroExpDist    double  m       sum of the distribution values
 * sumxx        HydroExpDist    double  m       sum of the squared distribution values
 *
 *
 *  Original:   July 1998   Mark Morehead
 */
#include "hydroclimate.h"
#include "hydroparams.h"
#include "hydrodaysmonths.h"

#ifdef DBG
    #include "hydroinout.h"
#endif

#define ntot (60)

/*---------------------------
 *  Start of HydroExpDist.c
 *---------------------------*/
int
hydroexpdist(double pvals[31], int mnth)
{

    double dumdbl, sumx, sumxx;
    double stda, stdb;
    double da[ntot], db[ntot], dc[ntot];
    int ii, kk, err;

    /*------------------------
     *  Initialize variables
     *------------------------*/
    err   = 0;
    sumx  = 0.0;
    sumxx = 0.0;

    /*----------------------------------------------------------------
     *  Transform more than the number of needed values
     *  this allows for the removal of outliers
     *  Pretend to be making a two sided distribution with mean == 0
     *  therefore: sumx == 0
     *----------------------------------------------------------------*/
    for (ii = 0; ii < ntot; ii++) {
        dumdbl = ranarray[nran];
        nran++;

        if (dumdbl < 0.0) {
            dumdbl = -dumdbl;
        }

        da[ii] = pow(exp(dumdbl), Pexponent[ep]);
        sumxx += sq(da[ii]);
    }

    /*-------------------------------------------------------------------------
     *  Calculate the standard deviation of the 1st round of numbers (da[ii])
     *-------------------------------------------------------------------------*/
    stda = sqrt(sumxx / ntot);

    /*-------------------------------------------------------------------
     *  Normalize the values to the input Standard Deviation (1st Pass)
     *  remove outliers (or save good values)
     *  find new Standard Deviation
     *-------------------------------------------------------------------*/
    sumx  = 0.0;
    sumxx = 0.0;
    kk    = 0;

    for (ii = 0; ii < ntot; ii++) {
        dumdbl = (Pmassbal[ep] * Pnomstd[mnth][ep] / stda) * da[ii];

        if (0 < dumdbl && dumdbl < Prange[ep]*Pmassbal[ep]*Pnomstd[mnth][ep]) {
            db[kk] = dumdbl;
            sumxx += sq(db[kk]);
            kk++;
        }
    }

    /*----------------------------------------------------------------------
     *  Calculate the Standard Deviation of the generated numbers (db[ii])
     *----------------------------------------------------------------------*/
    stdb = sqrt(sumxx / kk);

    /*-------------------------------------------------------------------
     *  Normalize the values to the input Standard Deviation (2nd Pass)
     *-------------------------------------------------------------------*/
    for (ii = 0; ii < kk; ii++) {
        dc[ii] = (Pmassbal[ep] * Pnomstd[mnth][ep] / stdb) * db[ii];
    }

    /*--------------------------------------------
     *  make sure we return enough usable points
     *--------------------------------------------*/
    //if( kk < daysim[mnth] ) {
    if (kk < days_in_month(mnth)) {
        fprintf(stderr,
            " HydroExpDist ERROR: Not enough points generated for the non-normal distribution.\n");
        fprintf(stderr, "    Increase NTOT in expdist.c \n");
        //   fprintf( stderr, "    epoch = %d, year = %d, month = %d, daysim = %d \n",ep+1,yr,mnth,daysim[mnth]);
        fprintf(stderr, "    epoch = %d, year = %d, month = %d, daysim = %d \n", ep + 1, yr,
            mnth, days_in_month(mnth));
        fprintf(stderr, "    started ntot \t = %d \n", ntot);
        fprintf(stderr, "    Generated (kk) \t = %d \n", kk);
        //   fprintf( stderr, "    needed daysim \t = %d \n", daysim[mnth]);
        fprintf(stderr, "    needed daysim \t = %d \n", days_in_month(mnth));
        err = 1;
    } else

        //  for( ii=0; ii<daysim[mnth]; ii++ )
        for (ii = 0; ii < days_in_month(mnth) ; ii++) {
            pvals[ii] = dc[ii];
        }


#ifdef DBG

    if (tblstart[ep] <= yr && yr <= tblend[ep] && mnth == 0) {
        fprintf(fidlog, " HydroExpDist:\n");
        //   fprintf( fidlog, "    epoch = %d, year = %d, month = %d, daysim = %d \n",ep+1,yr,mnth+1,daysim[mnth]);
        fprintf(fidlog, "    epoch = %d, year = %d, month = %d, daysim = %d \n", ep + 1, yr,
            mnth + 1, days_in_month(mnth));
        fprintf(fidlog, "    started ntot \t = %d \n", ntot);
        fprintf(fidlog, "    Generated (kk) \t = %d \n", kk);
        //   fprintf( fidlog, "    needed daysim \t = %d \n", daysim[mnth]);
        fprintf(fidlog, "    needed daysim \t = %d \n", days_in_month(mnth));
        sumx = 0.0;

        if (0) {
            //      for( ii=0; ii<daysim[mnth]; ii++) {
            for (ii = 0; ii < days_in_month(mnth); ii++) {
                fprintf(fidlog, "    ii = %d, \t pvals[ii] = %f \n", ii, pvals[ii]);
                sumx += pvals[ii];
            }

            fprintf(fidlog, "    sum(pvals) \t = %f \n\n", sumx);
        }
    }

#endif

    return (err);
} /* end of HydroExpdDist.c */
