/*
 *  HydroTrend.c
 *
 *  Improved version of HydroTrend (River)
 *  Main program, controls flow, counts years...
 *  Converted to C, streamlined the code ...
 *  See the file FLOWCHART for code details.
 *
 *  Author:    M.D. Morehead (June 1998)
 *  Author2:   S.D. Peckham  (September 2001)
 *  Author3:   A.J. Kettner (August-October 2002) (February-April 2003)
 *
 *
 * Variable     Def.Location    Type        Units   Usage
 * --------     ------------    ----        -----   -----
 * **argv       HydroTrend.c    char        -       command line string capture variable
 * argc         HydroTrend.c    int         -       command line word counter
 * err          various         int         -       error flag, halts program
 * dumdbl       various         double      -       temporary double
 * i            various         int         -       temporary loop counter
 * ii           various         int         -       temporary loop counter
 * jj           various         int         -       temporary loop counter
 * setstartmeanQandQs HydroParams.h int     -       temporary loop counter
 * logarea      HydroTrend.c    double      m^2     log10 of the basin area
 * lyear        HydroTrend.c    int         yr      last year of an epoch
 * maxnran      HydroTrend.c    int         -       max # of random number used/year
 * pst[TMLEN]   HydroTrend.c    char        -       time stamp
 * Qgrandtotaltot HydroTrend.c  double      m^3/s   discharge for all run
 * Qgrandtotalperepoch HydroTrend.c double  m^3/s   discharge per epoch
 * Qpeakmax     HydroTrend.c    double      m^3/s   all time max. peak for all run
 * Qsbarnewtot  HydroTrend.c    double      kg/s    total of all Qsbarnew's
 * Qsgrandtotaltot HydroTrend.c double      kg/s    total of all Qsbrandtotal's
 * tloc         HydroTrend.c    struct      -       time stamp structure
 * tm timept    HydroTrend.c    struct      -       time stamp structure
 *
 */

#include "hydrofree_mem.h"
#include "hydroclimate.h"
#include "hydroinout.h"
#include "hydroparams.h"
#include "hydrotimeser.h"
#include "hydroalloc_mem.h"
#include "hydroreadclimate.h"
#include "hydrotrend.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

/*---------------------
 *  Start the program
 *---------------------*/
int
main(int argc, char** argv)
{

    /*-------------------
     *  Local Variables
     *-------------------*/
    char    pst[TMLEN];
    time_t  tloc;
    struct  tm* timeptr;
    int     err, ii, lyear, maxnran, verbose, p, k, x;
    double  logarea;
    long totaldays;
    gw_rainfall_etc* gw_rain;

    /*------------------------
     *  Initialize Variables
     *------------------------*/
    err             = 0;
    verbose         = 0;
    maxnran         = 0;
    maxerr          = 0.0;
    totalmass       = 0.0;
    globalparflag   = 0;
    Qgrandtotaltot  = 0.0;
    Qsgrandtotaltot = 0.0;
    TEtot           = 0.0;
    yeartot         = 0.0;
    gw_rain = (gw_rainfall_etc*)malloc(sizeof(gw_rainfall_etc));

    /*----------------------
     *  Get the start time
     *----------------------*/
    time(&tloc);
    timeptr = localtime(&tloc);

    /*--------------------------------
     *  Set the hardwired parameters
     *--------------------------------*/
    if (verbose) {
        printf("Setting hardwired parameters... \n");
    }

    err = hydrosetparams();

    if (err) {
        fprintf(stderr, " ERROR in HydroSetParams: HydroTrend Aborted \n\n");
        exit(1);
    }

    /*-----------------------------------------------------------------
     *  Check the command line input; get file name or directory name
     *-----------------------------------------------------------------*/
    if (verbose) {
        printf("check command line parameters... \n");
    }

    err = hydrocommandline(&argc, argv);

    if (err) {
        fprintf(stderr, " ERROR in HydroCommandLine: HydroTrend Aborted \n\n");
        exit(1);
    }

    /*--------------------------------------------
     *  Security Check, check input files on any
     *  characters which are not allowed.
     *  Set pathnames right for web directories.
     *--------------------------------------------*/
    if (webflag == 1) {
        if (verbose) {
            printf("Checking input files for security reasons... \n");
        }

        err = hydrosecurityinputcheck();

        if (err) {
            fprintf(stderr,
                "  Error in HydroSecurityInputCheck: Unable to continue due to security reasons.\n");
            fprintf(stderr,
                "    The program found %d character(s) which you are not allowed to use.\n", err);
            fprintf(stderr, "    Get ride of the following characters and try it again.\n");

            for (ii = 0; ii < err; ii++) {
                fprintf(stderr, "    %c\n", chrdump[ii]);
            }

            fprintf(stderr, "    program aborted. \n");
            fclose(fidinput);

            for (ep = 0; ep < nepochs; ep++) {
                fclose(fidhyps[ep]);
            }

            exit(1);
        }
    }

    /*--------------------------------
     *  Read the main input file.
     *  This reads all of the epochs,
     *  at the very begining
     *--------------------------------*/
    if (verbose) {
        printf("Reading input data from: HYDRO.IN... \n");
    }

    err = hydroreadinput();

    if (err) {
        fprintf(stderr, " ERROR in HydroReadInput: HydroTrend Aborted \n\n");
        exit(1);
    }

    /*---------------------------------------
     *  Set the output path name + filename
     *---------------------------------------*/
    strcpy(startname, directory);

    if (argc == 3) {
        strcat(startname, commandlinearg[2]);
    }

    strcat(startname, commandlinearg[1]);

    /*---------------------
     *  Open the log file
     *---------------------*/
    if (verbose) {
        printf("Opening the log file (to append)... \n");
    }

    strcpy(ffnamelog, startname);
    strcat(ffnamelog, fnamelog);

    if ((fidlog = fopen(ffnamelog, "w+")) == NULL) {
        printf("  HydroTrend WARNING: Unable to open the log file %s \n", ffnamelog);
        printf("     non-fatal error, continuing. \n\n");
    }

    /*--------------------------------
     *  Print the program start time
     *--------------------------------*/
    strftime(pst, TMLEN, "%X  %x", timeptr);
    fprintf(fidlog, " ======================================================== \n\n");
    fprintf(fidlog, " ----- HydroTrend 3.0 Model Run ----- \n\n");
    fprintf(fidlog, " Start: %s \n\n", pst);

    /*---------------------------------
     *  Read the hypsometric curve
     *  and set maxalt and totalarea.
     *---------------------------------*/
    if (verbose) {
        printf("Reading hypsometric data from: HYDRO.HYPS... \n");
    }

    err = hydroreadhypsom();

    if (err) {
        fprintf(stderr, " ERROR in HydroReadHypsom: HydroTrend Aborted \n\n");
        fprintf(fidlog, " ERROR in HydroReadHypsom: HydroTrend Aborted \n\n");
        exit(1);
    }

    /*---------------------------------------------
     *  Read climate files if they are their
     *  For now the climate files are only usable
     *  when topmodel gw module is turned on (is that so??)
     *---------------------------------------------*/
    if (verbose) {
        printf("Reading Topmodel Groundwater data files... \n");
    }

    err = hydroreadclimate(gw_rain);

    if (err) {
        fprintf(stderr, " ERROR in HydroReadclimate: HydroTrend Aborted \n\n");
        fprintf(fidlog, " ERROR in HydroReadclimate: HydroTrend Aborted \n\n");
        exit(1);
    }

    /*------------------------------------------
     *  Set global values for those parameters
     *  which doesn't have any input
     *------------------------------------------*/
    if (globalparflag > 0) {
        if (verbose) {
            printf("Set global values for not filed out input parameters... \n");
        }

        err = hydrosetglobalpar();

        if (err) {
            fprintf(stderr, " ERROR in HydroSetGlobalPar: HydroTrend Aborted \n\n");
            fprintf(fidlog, " ERROR in HydroSetGlobalPar: HydroTrend Aborted \n\n");
            exit(1);
        }
    }

    /*---------------------------------------------
     *  Check all of the input values.
     *  This also checks to make sure the climate
     *  variables match at the epoch breaks.
     *---------------------------------------------*/
    if (verbose) {
        printf("Checking all input parameters... \n");
    }

    err = hydrocheckinput();

    if (err) {
        fprintf(stderr, " ERROR in HydroCheckInput: HydroTrend Aborted \n\n");
        fprintf(fidlog, " ERROR in HydroCheckInput: HydroTrend Aborted \n\n");
        exit(1);
    }

    /*-----------------------
     *  Open the data files
     *-----------------------*/
    if (verbose) {
        printf("Opening output data files... \n");
    }

    err = hydroopenfiles();

    if (err) {
        fprintf(stderr, " ERROR in HydroOpenFiles: HydroTrend Aborted \n\n");
        fprintf(fidlog, " ERROR in HydroOpenFiles: HydroTrend Aborted \n\n");
        exit(1);
    }

    if (verbose) {
        printf(" \nStarting epoch loop... \n");
    }

    /*--------------------------
     *  Run each epoch of data
     *--------------------------*/
    for (ep = 0; ep < nepochs; ep++) {
        total_yr += nyears[ep];
        ranarray = malloc1d(2 * maxran, double);

        /*-----------------------------------------------------------------
         *  Read Qs constant parameters set by geolocation of river mouth
         *-----------------------------------------------------------------*/
        if (verbose) {
            printf("Calling HydroSetGeoParams... \n");
        }

        err = hydrosetgeoparams(gw_rain);

        if (err) {
            fprintf(stderr, " ERROR in HydroSetGeoParams: HydroTrend Aborted \n\n");
            fprintf(fidlog, " ERROR in HydroSetGeoParams: HydroTrend Aborted \n\n");
            exit(1);
        }

        /*----------------------------------------------------------------------------
         *  Run each epoch 4 times; once to calculate the mean discharge (Qbar),
         *  once to calculate the mean sediment discharge (Qsbarnew), and once to
         *  calculate the daily sediment discharge. WHATS DONE THE 4TH TIME?
         *----------------------------------------------------------------------------*/
        for (setstartmeanQandQs = 0; setstartmeanQandQs < 5; setstartmeanQandQs++) {
            yr = syear[ep];

            /*---------------------------------------------------
             *  Free memory for possible multiple outlet module
             *---------------------------------------------------*/
            if ((ep > 0 && setstartmeanQandQs == 0)) {
                hydrofreememoutlet(nyears[ep - 1]);
            }

            if ((ep > 0 && setstartmeanQandQs == 0)) {
                hydrofreememoutlet1(ep);
            }

            /*-------------------------------------------------------
             *  Allocate memory for possible multiple outlet module
             *-------------------------------------------------------*/
            if (setstartmeanQandQs == 0) {
                hydroallocmemoutlet(ep);
            }

            if (setstartmeanQandQs == 1) {
                hydroallocmemoutlet1(ep);
            }

            /*----------------------------------
             *  Initialize Variables per epoch
             *----------------------------------*/
            if ((nooutletpctflag == 0) && (setstartmeanQandQs > 0)) {
                for (p = 0; p < maxnoutlet; p++) {
                    Qdummy[ep][p] = 0.0;

                    for (k = 0; k < eventsnr[ep]; k++) {
                        Qbar[ep][p][k]   = 1.0;     /* Just a start value */
                        outletpct[p][ep][k] = outletpctdummy[p][ep];
                    }
                }
            }

            if (setstartmeanQandQs == 0)
                for (p = 0; p < nyears[ep]; p++)
                    for (k = 0; k < daysiy; k++) {
                        Qpeakfloodtemp[p][k] = 0.0;
                    }

            /*-----------------------------------------------------------------
             *  Start new random number sequence.
             *  Get 'maxran' worth of random numbers and pluck them as needed
             *  nran counts through the numbers stored in ranarray
             *-----------------------------------------------------------------*/
            if (verbose) {
                printf("Calling HydroRandom... \n");
            }

            err = hydrorandom();

            if (err) {
                fprintf(stderr, " ERROR in HydroRandom: HydroTrend Aborted \n\n");
                fprintf(fidlog, " ERROR in HydroRandom: HydroTrend Aborted \n\n");
                exit(1);
            }

            nran = 0;

            /*-------------------------------------------------
             *  Initialize Variables per loop through program
             *-------------------------------------------------*/
            Qpeakall[ep] = 0.0;
            eventcounter = 0;

            for (p = 0; p < maxnoutlet; p++) {
                Qpeakperoutletall[ep][p] = 0.0;
            }

            if (Qsbarformulaflag[ep] == 1 && setstartmeanQandQs == 1) {
                setstartmeanQandQs++;
            }

            /*-------------------------------------------
             *  Set the number of outlets for 10 events
             *  if number of outlets is not specified,
             *  or given in a range.
             *-------------------------------------------*/
            if (noutletflag == 1 && setstartmeanQandQs == 1) {
                for (x = 0; x < eventsnr[ep]; x++) {
                    nroutlets[x] = hydrosetnumberoutlet(x);
                    noutlet = nroutlets[x];
                    err = hydrooutletfraction(x);

                    if (err) {
                        fprintf(stderr, " ERROR in HydroOutletFraction (HydroOutlet): HydroTrend Aborted \n\n");
                        fprintf(fidlog, " ERROR in HydroOutletFraction (HydroOutlet): HydroTrend Aborted \n\n");
                        exit(1);
                    }
                }

                noutlet = nroutlets[eventcounter];
            }

            if (noutletflag == 0 && outletmodelflag == 1 && steadyoutletpctflag == 1
                && setstartmeanQandQs == 1)
                for (x = 0; x < eventsnr[ep]; x++) {
                    nroutlets[x] = noutlet;
                    err = hydrooutletfraction(x);

                    if (err) {
                        fprintf(stderr, " ERROR in HydroOutletFraction (HydroOutlet): HydroTrend Aborted \n\n");
                        fprintf(fidlog, " ERROR in HydroOutletFraction (HydroOutlet): HydroTrend Aborted \n\n");
                        exit(1);
                    }
                }

            /*-----------------------------------------------------------------------
             *  Set the discharge fraction per outlet if there are multiple outlets
             *  and if the discharge fraction is not set yet in the outlet file
             *-----------------------------------------------------------------------*/
            if (outletmodelflag == 1 && nooutletpctflag == 1 && steadyoutletpctflag == 0
                && setstartmeanQandQs == 1) {
                x = 0;
                err = hydrooutletfraction(x);

                if (err) {
                    fprintf(stderr, " ERROR in HydroOutletFraction (HydroOutlet): HydroTrend Aborted \n\n");
                    fprintf(fidlog, " ERROR in HydroOutletFraction (HydroOutlet): HydroTrend Aborted \n\n");
                    exit(1);
                }
            }

            /*---------------------------------------------------------
             *  Calculate the maximum predicted flood size.
             *  Since the basin area does not change within an epoch,
             *  only need to do this once for each epoch.
             *
             *  Maximum Flood Size for a Basin:
             *  Mulder T. and Syvitski J.P.M., 1995.
             *  Turbidity currents generated at river mouths
             *  during exceptional discharge to the world oceans.
             *  Journal of Geology, 103: 285-298.
             *
             *  The equation wants area in km^2
             *
             *---------------------------------------------------------*/
            if (verbose) {
                printf("  Epoch = %d \n", ep + 1);
            }

            logarea  = log10(totalarea[ep] / 1e6);
            maxflood = pow(10.0, (2.084 + 0.865 * logarea - 0.07 * sq(logarea)));

#ifdef DBG
            fprintf(stderr, "\n ======================================= \n");
            fprintf(stderr, "\n HydroTrend:  epoch = %d \n\n", ep + 1);
            fprintf(stderr, "\t HydroShoulder: maxflood \t = %g \n\n", maxflood);
            fprintf(fidlog, "\n ======================================= \n");
            fprintf(fidlog, "\n HydroTrend:  epoch = %d \n\n", ep + 1);
            fprintf(fidlog, "\t HydroTrend: maxflood \t = %g \n\n", maxflood);
#endif

            /*---------------------------------------------------
             *  Create flood wave attenuation "shoulder" params
             *---------------------------------------------------*/
            if (verbose) {
                printf("Calling HydroShoulder... \n");
            }

            err = hydroshoulder();

            if (err) {
                fprintf(stderr, " ERROR in HydroShoulder: HydroTrend Aborted \n\n");
                fprintf(fidlog, " ERROR in HydroShoulder: HydroTrend Aborted \n\n");
                exit(1);
            }

            /*---------------------------------------
             *  Loop through each year of the epoch
             *---------------------------------------*/
            lyear = syear[ep] + nyears[ep];

            for (yr = syear[ep]; yr < lyear; yr++) {

                /*-------------------------------------------------------------
                 *  Reset annual arrays tracking carryover from previous year
                 *-------------------------------------------------------------*/
                if (ep > 0 || yr > syear[ep])
                    for (ii = 0; ii < maxday - daysiy; ii++) {
                        Qrainwrap[ii]  = Qrain[ii + daysiy];
                        Qicewrap[ii]   = Qice[ii + daysiy];
                        Qnivalwrap[ii] = Qnival[ii + daysiy];
                        Qsswrap[ii]    = Qnival[ii + daysiy];
                    } else
                    for (ii = 0; ii < maxday - daysiy; ii++) {
                        Qrainwrap[ii]  = 0.0;
                        Qicewrap[ii]   = 0.0;
                        Qnivalwrap[ii] = 0.0;
                        Qsswrap[ii]    = 0.0;
                    }

                /*---------------------------------------
                 *  Keep track of groundwater pool size
                 *---------------------------------------*/
                gwlast = gwstore[daysiy - 1];

                /*--------------------------------------------------
                 *  In case the model exceeds the maximum flood,
                 *  loop through a number of times.  This normally
                 *  gets the maximum modeled flood to be below
                 *  the maximum predicted flood.
                 *--------------------------------------------------*/
                exceedflood = 1;
                floodtry = 0;

                while (exceedflood > 0 && floodtry < 10) {

                    /*------------------------------
                     *  Reset annual arrays/values
                     *------------------------------*/
                    for (ii = 0; ii < maxday; ii++) {
                        Qrain[ii]   = 0.0;
                        Qice[ii]    = 0.0;
                        Qnival[ii]  = 0.0;
                        Qss[ii] = 0.0;
                        Qsumtot[ii] = 0.0;

                        for (p = 0; p < maxnoutlet; p++) {
                            Qsum[ii][p] = 0.0;
                        }
                    }

                    for (ii = 0; ii < daysiy; ii++) {
                        if (outletmodelflag == 1)
                            for (p = 0; p < maxnoutlet; p++) {
                                Csoutlet[ii][p]     = 0.0;
                                Qboutlet[ii][p]     = 0.0;
                                Qsoutlet[ii][p]     = 0.0;
                            }

                        Cs[ii] = 0.0;
                        Qb[ii] = 0.0;
                        Qs[ii] = 0.0;
                        Qicetogw[ii]    = 0.0;
                        Qnivaltogw[ii]  = 0.0;
                        Pdaily[ii]      = 0.0;
                        Tdaily[ii]      = 0.0;
                        gwstore[ii]     = 0.0;
                    }

                    Enivalannual = 0.0;
                    Eiceannual   = 0.0;

                    /*---------------------------
                     *  Set the initial GW pool
                     *---------------------------*/
                    if (yr == syear[0]) {
                        gwstore[0] = gwinitial;
                        gwlast = gwinitial;
                    }

                    if (yr != syear[0]) {
                        gwstore[0] = gwlast;
                    }

                    /*-----------------------------------------------------------------
                     *  Start new random number sequence.
                     *  Get 'maxran' worth of random numbers and pluck them as needed
                     *  nran counts through the numbers stored in ranarray
                     *-----------------------------------------------------------------*/
                    rmin = -6.0;

                    while (rmin < -5.0 || rmax > 5.0) {
                        if (verbose) {
                            printf("Calling HydroRandom... \n");
                        }

                        err = hydrorandom();

                        if (err) {
                            fprintf(stderr, " ERROR in HydroRandom: HydroTrend Aborted \n\n");
                            fprintf(fidlog, " ERROR in HydroRandom: HydroTrend Aborted \n\n");
                            exit(1);
                        }
                    }

                    /*---------------------------------
                     *  Set the climate for this year
                     *---------------------------------*/
                    if (verbose) {
                        printf("Calling HydroClimate... \n");
                    }

                    err = hydroclimate(gw_rain);

                    if (err) {
                        fprintf(stderr, " ERROR in HydroClimate: HydroTrend Aborted \n\n");
                        fprintf(fidlog, " ERROR in HydroClimate: HydroTrend Aborted \n\n");
                        exit(1);
                    }

#ifdef DBG
                    fprintf(stderr, " HydroTrend: \t Pannual = %f, \t Tannual = %f \n", Pannual, Tannual);
#endif

                    /*-------------------------------------
                     *  Calculate weather for each day of
                     *  the year, for each hypsometric bin
                     *-------------------------------------*/
                    if (verbose) {
                        printf("Calling HydroWeather... \n");
                    }

                    err = hydroweather(gw_rain);

                    if (err) {
                        fprintf(stderr, " ERROR in HydroWeather: HydroTrend Aborted \n\n");
                        fprintf(fidlog, " ERROR in HydroWeather: HydroTrend Aborted \n\n");
                        exit(1);
                    }

                    /*-------------------------------------------------
                     *  Calculate elev grid and T, for each elevation
                     *-------------------------------------------------*/
                    if (verbose) {
                        printf("Calling HydroHypsom... \n");
                    }

                    err = hydrohypsom();

                    if (err) {
                        fprintf(stderr, " ERROR in HydroHypsom: HydroTrend Aborted \n\n");
                        fprintf(fidlog, " ERROR in HydroHypsom: HydroTrend Aborted \n\n");
                        exit(1);
                    }

                    /*-------------------------------------------------
                     *  Calculate ice accumulation/melt for each day.
                     *  This is done before HydroRain or HydroSnow to
                     *  find the glaciated area
                     *-------------------------------------------------*/
                    if (verbose) {
                        printf("Calling HydroGlacial... \n");
                    }

                    err = hydroglacial();

                    if (err) {
                        fprintf(stderr, " ERROR in HydroGlacial: HydroTrend Aborted \n\n");
                        fprintf(fidlog, " ERROR in HydroGlacial: HydroTrend Aborted \n\n");
                        exit(1);
                    }

                    /*------------------------------------------
                     *  Calculate snow fall/melt for each day.
                     *  This is done before HydroRain to find
                     *  the "snow" area for each day
                     *------------------------------------------*/
                    if (verbose) {
                        printf("Calling HydroSnow... \n");
                    }

                    err = hydrosnow();

                    if (err) {
                        fprintf(stderr, " ERROR in HydroSnow: HydroTrend Aborted \n\n");
                        fprintf(fidlog, " ERROR in HydroSnow: HydroTrend Aborted \n\n");
                        exit(1);
                    }

                    /*---------------------------------
                     *  Calculate precip for each day
                     *---------------------------------*/
                    if (verbose) {
                        printf("Calling HydroRain... \n");
                    }

                    err = hydrorain();

                    if (err) {
                        fprintf(stderr, " ERROR in HydroRain: HydroTrend Aborted \n\n");
                        fprintf(fidlog, " ERROR in HydroRain: HydroTrend Aborted \n\n");
                        exit(1);
                    }

                    /*------------------------------------------------------------
                     *  Add the component flows and find peakflow for the year.
                     *  Store the lagged overflow and groundwater pool size for
                     *  the following year.
                     *------------------------------------------------------------*/
                    if (verbose) {
                        printf("Calling HydroSumFlow... \n");
                    }

                    err = hydrosumflow();

                    if (err) {
                        fprintf(stderr, " ERROR in HydroSumFlow: HydroTrend Aborted \n\n");
                        fprintf(fidlog, " ERROR in HydroSumFlow: HydroTrend Aborted \n\n");
                        exit(1);
                    }

                    /*-----------------------------------------
                     *  Is flood peak less than max allowed ?
                     *-----------------------------------------*/
                    if (Qpeak < maxflood) {
                        exceedflood = 0;
                    } else {
                        if (setstartmeanQandQs == 4) {
                            fprintf(stderr, "\n FLOOD WARNING: epoch %d, year %d \n", ep + 1, yr);
                            fprintf(stderr, " \t Max.Allowed %.1f, Qpeak %.1f, retry # %d \n", maxflood, Qpeak,
                                floodtry);
                        }

                        floodtry++;

                        if (floodtry < 10) {
                            Qgrandtotal[ep] -= Qtotal;
                        }
                    }
                }  /* end flood exceedence while loop */

                /*-------------------------------------------------------
                 *  Track the max flow and if we still exceed the max
                 *  predicted flood, send warning flag, but keep going
                 *-------------------------------------------------------*/
                Qpeakall[ep] = mx(Qpeak, Qpeakall[ep]);

                if (outletmodelflag == 1)
                    for (p = 0; p < maxnoutlet; p++) {
                        Qpeakperoutletall[ep][p] = mx(Qpeakperoutlet[p], Qpeakperoutletall[ep][p]);
                    }

                if (exceedflood > 0 && setstartmeanQandQs == 4) {
                    fprintf(stderr, "   FLOOD WARNING: the maximum predicted flood size");
                    fprintf(stderr, " has been exceeded. \n");
                    fprintf(stderr, "      Epoch %d, year %d \n", ep + 1, yr);
                    fprintf(stderr, "      Maximum predicted flood %g (m^3/s) \n", maxflood);
                    fprintf(stderr, "      Modeled flood peak %g (m^3/s) \n", Qpeak);

                    fprintf(fidlog, "   FLOOD WARNING: the maximum predicted flood size");
                    fprintf(fidlog, " has been exceeded. \n");
                    fprintf(fidlog, "      Epoch %d, year %d \n", ep + 1, yr);
                    fprintf(fidlog, "      Maximum predicted flood %g (m^3/s) \n", maxflood);
                    fprintf(fidlog, "      Modeled flood peak %g (m^3/s) \n", Qpeak);
                }

                /*-------------------------------------------------------------
                 *  Calculate the maximal events = (biggest Qpeaks per epoch)
                 *  for channel switching at delta if option is turned on
                 *------------------------------------------------------------- */
                if (setstartmeanQandQs == 1 && eventnrflag == 0) {
                    if (verbose) {
                        printf("Calling HydroMaxEvents... \n");
                    }

                    err = hydromaxevents();

                    if (err) {
                        fprintf(stderr, " ERROR in HydroMaxEvents: HydroTrend Aborted \n\n");
                        fprintf(fidlog, " ERROR in HydroMaxEvents: HydroTrend Aborted \n\n");
                        exit(1);
                    }
                }

                /*---------------------------
                 *  Calculate sediment load
                 *---------------------------*/
                if (setstartmeanQandQs > 1) {
                    if (verbose) {
                        printf("Calling HydroSedLoad... \n");
                    }

                    err = hydrosedload(gw_rain);

                    if (err) {
                        fprintf(stderr, " ERROR in HydroSedLoad: HydroTrend Aborted \n\n");
                        fprintf(fidlog, " ERROR in HydroSedLoad: HydroTrend Aborted \n\n");
                        exit(1);
                    }
                }

                /*------------------------------------------
                 *  Output the binary daily discharge file
                 *------------------------------------------*/
                if (setstartmeanQandQs == 4) {
                    if (verbose) {
                        printf("Calling HydroOutput... \n");
                    }

                    err = hydrooutput();

                    if (err) {
                        fprintf(stderr, " ERROR in HydroOutput: HydroTrend Aborted \n\n");
                        fprintf(fidlog, " ERROR in HydroOutput: HydroTrend Aborted \n\n");
                        exit(1);
                    }

                    /*------------------------------------------
                     *  Output the text file of tabulated data
                     *------------------------------------------*/

                    if (verbose) {
                        printf("Calling HydroPrintTable... \n");
                    }

                    if (tblstart[ep] <= yr && yr <= tblend[ep]) {
                        err = hydroprinttable();

                        if (err) {
                            fprintf(stderr, " WARNING in HydroPrintTable: Continuing \n\n");
                            fprintf(fidlog, " WARNING in HydroPrintTable: Continuing \n\n");
                        }
                    }

                    /*------------------------------------------------
                     *  Print out annual values to trend (trn) files
                     *------------------------------------------------*/
                    if (verbose) {
                        printf("Calling HydroPrintAnnual... \n");
                    }

                    err = hydroprintannual();

                    if (err) {
                        fprintf(stderr, " WARNING in HydroPrintTrend: Continuing \n\n");
                        fprintf(fidlog, " WARNING in HydroPrintTrend: Continuing \n\n");
                    }
                }  /* end setstartmeanQandQs == 4 statement */

                /*-------------------------------------------------------
                 *  Did random number generator create enough values ?
                 *-------------------------------------------------------*/
                if (nran >= maxran) {
                    fprintf(stderr, "\n\n HydroTrend ERROR: nran exceeded maxran.\n");
                    fprintf(stderr, "\t increase maxran in HydroParams.h. \n");
                    fprintf(stderr, "\t nran = %d, maxran = %d \n\n", nran, maxran);
                    exit(1);
                }

#ifdef DBG
                maxnran = mx(nran, maxnran);
                fprintf(stderr, " HydroTrend: \t nran = %d, maxnran = %d, out of maxran = %d \n", nran,
                    maxnran, maxran);
                fprintf(fidlog, "\n HydroTrend: nran = %d, maxnran = %d, out of maxran = %d \n", nran,
                    maxnran, maxran);
#endif
            }  /* end year loop */

            /*------------------------------------------
             *  Calculate Qbartotal[ep] (before the
             *  river might split in multiple outlets)
             *------------------------------------------*/
            if (setstartmeanQandQs == 0) {
                Qbartotal[ep] = Qgrandtotal[ep] / (daysiy * nyears[ep] * dTOs);

                if (eventnrflag == 1) {
                    if (floodcounter == 0) {
                        eventsnr[ep] = 1;
                    }

                    if (floodcounter > 0) {
                        floodcounter = 0;

                        for (p = 0; p < nyears[ep]; p++)
                            for (k = 0; k < daysiy; k++)
                                if (Qpeakfloodtemp[p][k] > 0.0) {
                                    Qpeakfloodtemp[p][k] = (Qpeakfloodtemp[p][k] * ((Qbartotal[ep] - baseflowtot[ep]) /
                                                Qbartotal[ep])) + baseflowtot[ep];

                                    if (Qpeakfloodtemp[p][k] > floodvalue[ep]) {
                                        floodcounter++;
                                    }
                                }

                        eventsnr[ep] = floodcounter;
                    }

                    eventnrflag = 0;
                }
            }

            /*-----------------------------------------
             *  Calculate Qbar[ep] per outlet, taking
             *  the possible events into acount
             *-----------------------------------------*/
            if (outletmodelflag == 1 && (setstartmeanQandQs == 1 || setstartmeanQandQs == 2))
                for (p = 0; p < maxnoutlet; p++)
                    for (k = 0; k < eventsnr[ep]; k++) {
                        if (daysievent[k] > 0) {
                            Qbar[ep][p][k] = (Qgrandtotaloutlet[ep][p][k]) / ((daysievent[k]) * dTOs);
                        } else {
                            Qbar[ep][p][k] = 0.0;
                        }
                    }

            // allocate memory
            if (eventnrflag == 0 && setstartmeanQandQs == 0) {
                hydroallocmemoutlet1(ep);
            }

            /*--------------------------------------------------
             *  Correction for baseflow on the peakevents. In
             *  hydrosumflow the correction will be calculated
             *  on a daily bases when setstartmeanQandQs > 0
             *--------------------------------------------------*/
            if ((noutletflag == 1 && setstartmeanQandQs == 0) || (noutletflag == 0
                    && setstartmeanQandQs == 0 && steadyoutletpctflag == 0))
                for (k = 0; k < eventsnr[ep]; k++) {
                    Qpeakallevents[ep][k] = ((Qpeakallevents[ep][k] * ((Qbartotal[ep] - baseflowtot[ep]) /
                                    Qbartotal[ep])) + baseflowtot[ep]);

                    /*--------------------------------------------
                     *  Shuffle Qfraction per outlet per event
                     *--------------------------------------------*/
                    nroutlets[k] = hydrosetnumberoutlet(k);
                    noutlet = nroutlets[k];
                    err = hydroqfractionshuffle(k);
                }

            /*---------------------------------------------
             *  Start Hydrocalqsnew to calc. and Qsbarnew
             *---------------------------------------------*/
            if (setstartmeanQandQs == 3) {
                if (verbose) {
                    printf("Calling Hydrocalqsnew... \n");
                }

                err = hydrocalqsnew();

                if (err) {
                    fprintf(stderr, " ERROR in Hydrocalqsnew: HydroTrend Aborted \n\n");
                    fprintf(fidlog, " ERROR in Hydrocalqsnew: HydroTrend Aborted \n\n");
                    exit(1);
                }

                fprintf(stderr, "\nCalculating sediment load for epoch nr. %d, \n", (ep + 1));

                if (outletmodelflag == 1) {
                    for (p = 0; p < maxnoutlet; p++) {
                        outletpcttotevents[p][ep] = 0.0;
                    }

                    for (p = 0; p < maxnoutlet; p++) {
                        totaldays = 0;

                        for (k = 0; k < eventsnr[ep]; k++) {
                            outletpcttotevents[p][ep] += outletpct[p][ep][k] * daysievent[k];
                            totaldays += daysievent[k];
                        }

                        outletpcttotevents[p][ep] /= (totaldays);
                    }
                }
            } /* end if setstartmeanQandQs == 3 */
        }    /* end for setstartmeanQandQs<5 loop */

        /*------------------------------------------------
         *  Set the variables for the summary statistics
         *------------------------------------------------*/
        if (outletmodelflag == 1) {
            totpercentageQ[ep] = 0.0;

            for (p = 0; p < maxnoutlet; p++) {
                totaldays = 0;

                for (k = 0; k < eventsnr[ep]; k++) {
                    totpercentageQ[ep] += outletpct[p][ep][k];
                    Qdummy[ep][p] += (Qbar[ep][p][k] * daysievent[k]);
                    outletpcttotevents[p][ep] += outletpct[p][ep][k] * daysievent[k];
                    totaldays += daysievent[k];
                }

                Qdummy[ep][p] /= totaldays;
                outletpcttotevents[p][ep] /= (totaldays);
                Qgrandtotaltotoutlet[p] = (Qgrandtotaloutlet[ep][p][k]);
                Qsgrandtotaltotoutlet[p] = Qsgrandtotaloutlet[ep][p];
                Qsgrandtotaldelta[ep] += Qsgrandtotaloutlet[ep][p];
                Coutlettotal[ep][p] = Coutlettotal[ep][p] / (daysiy * nyears[ep]);
            }

            if ((outletmodelflag == 1 && steadyoutletpctflag == 0) || (noutletflag == 1
                    && steadyoutletpctflag == 1) || (noutletflag == 0 && steadyoutletpctflag == 1)) {
                ep = 0;
                totpercentageQ[ep] /= eventsnr[ep];
            }
        } else {
            totpercentageQ[ep] = 1.0;
        }

        Qsgrandtotaltot += Qsgrandtotal[ep];
        Qgrandtotaltot += Qgrandtotal[ep];
        yeartot += daysiy * nyears[ep] * dTOs;

        if (nepochs == 1) {
            Qpeakmax = Qpeakall[ep];
        } else {
            Qpeakmax = mx(Qpeakmax, Qpeakall[ep]);
        }

        TEtot += TE[ep];

        /*------------------------------------------------------------------
         *  Print out statistic file, for values used in Qs and Q formulas
         *------------------------------------------------------------------*/
        if (verbose) {
            printf("Calling HydroprintStat... \n");
        }

        err = hydroprintstat();

        if (err) {
            fprintf(stderr, " WARNING in HydroPrintStat: Continuing \n\n");
            fprintf(fidlog, " WARNING in HydroPrintStat: Continuing \n\n");
        }

        freematrix1D((void*) ranarray);
        freematrix1D((void*) C);
    }  /* end epoch loop */

    /*---------------------------------
     *  Print some summary statistics
     *---------------------------------*/
    fprintf(stderr, "\n\nSTATISTICS PER EPOCH PER OUTLET\n");
    fprintf(stderr,
        "Ep\tOutlet\tQbar\tQsbar\tQsmean/outlet\tQpeak\tCsmean\tTEbas.TEres.\n");

    for (ep = 0; ep < nepochs; ep++) {
        p = 0;
        fprintf(stderr, "%d\t%d(%.0f%%)\t%.2f\t%.2f\t\t\t%.2f\t%.2f\t%.1f   %.1f\n",
            (ep + 1), p, totpercentageQ[ep] * 100, Qbartotal[ep], Qsbartot[ep], Qpeakall[ep],
            Csgrandtotal[ep] / (daysiy * nyears[ep]*dTOs), TE[ep] * 100, TEsubbasin[ep] * 100);

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++)
                fprintf(stderr, "%d\t%d(%.0f%%)\t%.2f\t%.2f\t% .2f%%\t\t%.2f\t%.2f\n",
                    (ep + 1), p + 1, outletpcttotevents[p][ep] * 100, Qdummy[ep][p],
                    Qsgrandtotaloutlet[ep][p] / (daysiy * nyears[ep]*dTOs),
                    (Qsgrandtotaloutlet[ep][p] / Qsgrandtotaldelta[ep]) * 100/*/(daysiy*nyears[ep]*dTOs)*/,
                    Qpeakperoutletall[ep][p], Csgrandtotaloutlet[ep][p] / (daysiy * nyears[ep]*dTOs));

        fprintf(stderr, "nr\tnr\t(m^3/s)\t(kg/s)\t(%%)\t\t(m^3/s)\t(kg/m3) (%%)   (%%)\n\n");
        fprintf(stderr, "Basin area                           = %.2f  (km^2) \n",
            totalarea[ep] / 1e6);
        fprintf(stderr, "Basin relief                         = %.2f  (m) \n\n\n", maxalt[ep]);
    }

    fprintf(fidlog, "\n\nSTATISTICS PER EPOCH PER OUTLET\n");
    fprintf(fidlog,
        "Ep\tOutlet\tQbar\tQsbar\tQsmean/outlet\tQpeak\tCsmean\tTEbas.TEres.\n");

    for (ep = 0; ep < nepochs; ep++) {
        p = 0;
        fprintf(fidlog, "%d\t%d(%.0f%%)\t%.2f\t%.2f\t\t\t%.2f\t%.2f\t%.1f   %.1f\n",
            (ep + 1), p, totpercentageQ[ep] * 100, Qbartotal[ep], Qsbartot[ep], Qpeakall[ep],
            Csgrandtotal[ep] / (daysiy * nyears[ep]*dTOs), TE[ep] * 100, TEsubbasin[ep] * 100);

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++)
                fprintf(fidlog, "%d\t%d(%.0f%%)\t%.2f\t%.2f\t% .1f%%\t\t%.2f\t%.2f\n",
                    (ep + 1), p + 1, outletpcttotevents[p][ep] * 100, Qdummy[ep][p],
                    Qsgrandtotaloutlet[ep][p] / (daysiy * nyears[ep]*dTOs),
                    (Qsgrandtotaloutlet[ep][p] / Qsgrandtotaldelta[ep]) * 100/*/(daysiy*nyears[ep]*dTOs)*/,
                    Qpeakperoutletall[ep][p], Csgrandtotaloutlet[ep][p] / (daysiy * nyears[ep]*dTOs));

        fprintf(fidlog, "nr\tnr\t(m^3/s)\t(kg/s)\t(%%)\t\t(m^3/s)\t(kg/m3) (%%)   (%%)\n\n");
        fprintf(fidlog, "Basin area                           = %.2f  (km^2) \n",
            totalarea[ep] / 1e6);
        fprintf(fidlog, "Basin relief                         = %.2f  (m) \n\n\n", maxalt[ep]);
    }

    /*-------------------------------
     *  Print the program stop time
     *-------------------------------*/
    time(&tloc);
    timeptr = localtime(&tloc);
    strftime(pst, TMLEN, "%X  %x", timeptr);
    fprintf(fidlog, "\n ------------------------- \n");
    fprintf(fidlog, "\n Stop: %19s \n", pst);
    fprintf(fidlog, " ------------------------- \n\n");

    /*-------------------
     *  Close all files
     *------------------*/
    fclose(fidq);
    fclose(fidqs);
    fclose(fidtrend1);
    fclose(fidtrend2);
    fclose(fidtrend3);
    fclose(fiddistot);
    fclose(fidlog);

    if (strncmp(asciioutput, ON, 2) == 0) {
        fclose(outp);
        fclose(outp1);
        fclose(outp2);
        fclose(outp3);
        fclose(outp4);
        fclose(outp5);
        fclose(outpnival_ice);
    }

    if (outletmodelflag == 1)
        for (p = 0; p < maxnoutlet; p++) {
            fclose(fiddis[p]);
        }

    /*-------------------------------------------------
     *  Swap big-endian and little-endian file format
     *-------------------------------------------------*/
    if (verbose) {
        printf("Calling HydroSwap... \n");
    }

    err = hydroswap();

    if (err) {
        fprintf(stderr, " WARNING in HydroSwap: Continuing \n\n");
    }

    /*-------------------------------------------
     *  Run CGI script for the internet version
     *-------------------------------------------*/
    if (webflag == 1) {
        char argstring[300];
        sprintf(argstring, "/lilac/htdocs/deltaforce/workshop/cgi-bin/returnmail.cgi %s",
            commandlinearg[2]);
        system(argstring);
    }

    /*---------------
     *  Free memory
     *---------------*/
    if (raindatafile == 1) {
        freematrix1D((void*) gw_rain->Tperyear);
    }

    /*---------------------------------------------------
     *  Free memory for possible multiple outlet module
     *---------------------------------------------------*/
    hydrofreememoutlet(ep);

    /*---------------------
     * Print exit message
     *---------------------*/
#ifdef DBG
    fprintf(stderr, " \n ---------------------------------------------------- \n");
    fprintf(stderr, "HydroTrend 3.0 finished. \n\n");
#else
    fprintf(stderr, "\nHydroTrend 3.0 finished. \n\n");
#endif
    return 0;
}  /* end of HydroTrend */
