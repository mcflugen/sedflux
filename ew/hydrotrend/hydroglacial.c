/*
 *  HydroGlacial.c
 *
 *  Calculates the daily ice accumulation or melt.
 *  Also calculates the groundwater flow and time lag.
 *
 *  Author1:    M.D. Morehead   (Original:   June 1998)
 *  Author2:    A.J. Kettner    (november 2003)
 *
 *  Variable    Def.Location    Type    Units   Usage
 *  --------    ------------    ----    -----   -----
 *  Mgw     HydroGlacial.c  double m^3/a Annual mass of Ice discharge going to GW
 * Mice     HydroGlacial.c  double  m^3/a   Annual mass of Ice derived discharge
 * Minput   HydroGlacial.c  double  m^3/a   Annual mass input into the Glacial routine
 * Mout     HydroGlacial.c  double  m^3/a   Annual mass output from the Glacial routine
 * Mwrap    HydroGlacial.c  double  m^3/a   Annual mass of discharge carried over from the previous year
 * Parea    HydroGlacial.c  double  m^2 daily area over which "ice" precipitation occurs
 * Tcorrection  HydroGlacial.c  double  degC    Temperature correction to ice melt applied for days with rain
 * Tfix     HydroGlacial.c  double  degC    Temperature correction for cold years
 * approxarea   HydroGlacial.c  double  m^2 the elevation binned area for the glacier
 * elabin   HydroGlacial.c  double  -   the closest elevbin to the ela
 * elaerror HydroGlacial.c  double  m   the height difference between the actual and binned ela
 * err      various     int -   error flag, halts program
 * glacierind   HydroGlacial.c  int -   the elevbin index at the toe of the glacier
 * ii       various     int -   temporary loop counter
 * indx     HydroGlacial.c  int -   temporary elevbin index
 * jj       various     int -   temporary loop counter
 * kk       various     int -   temporary loop counter
 * massavailable    HydroGlacial.c  double  m^3 total water available for E,Q and Gw=sum(ice balance+Precip)
 * maxmelt  HydroGlacial.c  double  degC    warmest melt day, used to prevent oversized floods
 * meltday[maxday]HydroGlacial.c    double  m^3 the amount of ice melt occuring on a given day
 * meltflag HydroGlacial.c  int -   flag to check for good ice melt years
 * shldday[maxday]HydroGlacial.c    double  m^3 the discharge going into shoulder events
 * smallgapprox HydroGlacial.c  double  m^2 binned glaciated area above the elabin
 * totalmelt    HydroGlacial.c  double  m^3 scale factor for distributing the melt events
 *
 */

#include <stdlib.h>
#include "hydroclimate.h"
#include "hydroparams.h"
#include "hydrotimeser.h"
#include "hydroinout.h"
#include "hydrodaysmonths.h"

/*-------------------------
 *  Start of HydroGlacial
 *-------------------------*/
int
hydroglacial()
{

    /*-------------------
     *  Local Variables
     *-------------------*/
#ifdef DBG
    FILE* fid;
#endif

    int err, ii, jj, kk, indx, glacierind, meltflag;
    int dumint;

    double elabin, elaerror, approxarea, Parea;
    double massavailable, maxmelt, meltday[maxday], shldday[maxday];
    double smallgapprox;
    double totalmelt, Tcorrection, Tfix, Mice, Mgw, Mout, Mwrap, Minput;
    double Tmean;
    double Volumelast, Volumeglacierarea;
    double lastareakm, glacierareakm;
    err = 0;
    glacierind = 0;
    elabin = 0.0;
    Tmean = 0.0;


    /*---------------------------------------------------------
     *  Approximate the ELA and glaciated area for the
     *  year prior to the model run.
     *  If (floodtry > 0) then keep that lastela and lastarea
     *  from the first time through
     ----------------------------------------------------------*/
    if (floodtry == 0) {
        if (ep == 0 && yr == syear[ep]) {
            /*----------------------------------------
             *  Find the closest elevbin to the ela;
             *  this may be above or below the ela
             *----------------------------------------*/
            if (ELAstart[ep] > maxalt[ep]) {
                lastela = ELAstart[ep] - ELAchange[ep];
                elaerror = 0.0;
                elabin = ELAstart[ep];
                ELAindex = 2 * nelevbins;
                smallg = 0.0;
                bigg = 0.0;
                lastarea = 0.0;
            } else {
                lastela  = ELAstart[ep] - ELAchange[ep];
                elaerror = maxalt[ep];

                for (kk = 0; kk < nelevbins; kk++)
                    if (fabs(lastela - elevbins[kk]) < elaerror) {
                        elabin = elevbins[kk];
                        ELAindex = kk;
                        elaerror = fabs(lastela - elevbins[kk]);
                    }

                /*-------------------------------------------------
                 *  Calculate the glaciated area above the elabin
                 *  by summing the areas above that point
                 *-------------------------------------------------*/
                smallgapprox = 0.0;

                for (kk = ELAindex; kk < nelevbins; kk++) {
                    smallgapprox += areabins[kk];
                }

                /*---------------------------------------------------
                 *  Determine which bin the ela actually resides in
                 *---------------------------------------------------*/
                if (elabin > lastela) {
                    indx = ELAindex - 1;
                } else {
                    indx = ELAindex;
                }

                /*--------------------------------------------
                 *  Correct the area by linear interpolation
                 *--------------------------------------------*/
                smallg = smallgapprox + areabins[indx] * (elabin - lastela) / elevbinsize;

                /*--------------------------------------------------------
                 * Calculate the glaciated area below the ela
                 * Assume area below ela = 35% of total area
                 * G = 0.35*Atotal  g = 0.65*Atotal    G = (0.35/0.65)*g
                 *--------------------------------------------------------*/
                bigg = (0.35 / 0.65) * smallg;

                /*------------------------------------
                 *  Find the estimate glaciated area
                 *------------------------------------*/
                lastarea = smallg + bigg;

            } /* end if ELA > maxalt */
        }

        /*-----------------------------------------------------------------
         *  Keep last years Glaciated area for Mass Balance and Discharge
         *-----------------------------------------------------------------*/
        else {
            if (ep != 0 && yr == syear[ep] && setstartmeanQandQs == 0) {
                initiallastela = ela;
                initiallastarea = glacierarea;
                lastela  = ela;
                lastarea = glacierarea;
            } else if (ep != 0 && yr == syear[ep] && setstartmeanQandQs > 0) {
                lastela = initiallastela;
                lastarea = initiallastarea;
            } else if (yr != syear[ep]) {
                lastela  = ela;
                lastarea = glacierarea;
            }
        }
    }   /* endif floodtry==0 */

    /*-------------------------
     *  Calculate the new ELA
     *-------------------------*/
    ela = ELAstart[ep] + ELAchange[ep] * ((yr - syear[ep]));

    /*-------------------------------------
     *  Simulate a basin with NO glaciers
     *-------------------------------------*/
    if (ela > maxalt[ep]) {
        if (lastela < maxalt[ep]) {
            fprintf(stderr, "\nHydroGlacial WARNING: epoch = %d, year = %d \n", ep + 1, yr);
            fprintf(stderr, "   The Glacier completely melted. \n");
            fprintf(stderr, "   This has not been accounted for yet. \n");
            fprintf(stderr, "   There will be a mass balance error for \n");
            fprintf(stderr, "   the remaining part of the glacier. \n");
        }

        glacierelev = maxalt[ep] + elevbinsize; /* make sure it is above the basin */
        glacierarea = 0.0;
        approxarea = 0.0;
        bigg = 0.0;
        smallg = 0.0;
        smallgapprox = 0.0;
    }

    /*----------------------------------
     *  Simulate a basin with glaciers
     *----------------------------------*/
    else {
        for (ii = 0; ii < maxday; ii++) {
            meltday[ii] = 0.0;
            shldday[ii] = 0.0;
        }

        /*----------------------------------------
         *  Find the closest elevbin to the ela;
         *  this may be above or below the ela
         *----------------------------------------*/
        elaerror = maxalt[ep];

        for (kk = 0; kk < nelevbins; kk++)
            if (fabs(ela - elevbins[kk]) < elaerror) {
                elabin = elevbins[kk];
                ELAindex = kk;
                elaerror = fabs(ela - elevbins[kk]);
            }

        /*--------------------------------------------------
         *    Calculate the glaciated area above the elabin
         *    by summing the areas above that point
         *--------------------------------------------------*/
        smallgapprox = 0.0;

        for (kk = ELAindex; kk < nelevbins; kk++) {
            smallgapprox += areabins[kk];
        }

        /*---------------------------------------------------
         *  Determine which bin the ela actually resides in
         *---------------------------------------------------*/
        if (elabin > ela) {
            indx = ELAindex - 1;
        } else {
            indx = ELAindex;
        }

        /*--------------------------------------------
         *  Correct the area by linear interpolation
         *--------------------------------------------*/
        smallg = smallgapprox + areabins[indx] * (elabin - ela) / elevbinsize;

        /*--------------------------------------------------------
         *    Calculate the glaciated area below the ela
         *    Assume area below ela = 35% of total area
         *    G = 0.35*Atotal     g = 0.65*Atotal    G = (0.35/0.65)*g
         *--------------------------------------------------------*/
        bigg = (0.35 / 0.65) * smallg;

        /*--------------------------------------------------
         *  Find the actual glaciated area (glacierarea)
         *  and elevation of the glacier toe (glacierelev)
         *  and glacierelev's elevbins index (glacierind)
         *--------------------------------------------------*/
        glacierarea = smallg + bigg;
        approxarea = 0.0;
        kk = nelevbins - 1;

        while (approxarea <= glacierarea) {
            approxarea += areabins[kk];
            glacierelev = elevbins[kk];
            glacierind = kk;
            kk--;
        }

        /*---------------------------------------------------------------------------
         *  Sum the Precip which is above BOTH:
         *   i) the ELA
         *   ii) the Freezing line altitude (FLAindex)
         *
         *   if( FLAindex == FLAflag ) then no T<0 occured on that day at any elev.
         *---------------------------------------------------------------------------*/
        MPglacial = 0.0;

        for (ii = 0; ii < daysiy; ii++) {
            /*  Find area above the ELA */
            if (FLAindex[ii] < ELAindex) {
                Parea = smallgapprox;
            }
            /* Find area above the FLA */
            else if (FLAindex[ii] < FLAflag) {
                Parea = 0.0;

                for (kk = nelevbins - 1; kk >= FLAindex[ii]; kk--) {
                    Parea += areabins[kk];
                }
            } else {
                Parea = 0.0;               /* FLA is above the basin */
            }

            MPglacial += Pdaily[ii] * Parea;
        }

        /*---------------------------------------------------------
         *  Track the actual changes in glacier mass so there are
         *  no step changes in mass between years
         *---------------------------------------------------------*/
        if (glacierarea < lastarea) {
            lastareakm = (lastarea / 1e6);
            glacierareakm = (glacierarea / 1e6);
            Volumelast = bethaglacier * pow(lastareakm, bethaexpo);
            Volumeglacierarea = bethaglacier * pow(glacierareakm, bethaexpo);
            Gmass = ((Volumelast * 1e6) - (Volumeglacierarea * 1e6));
        } else {
            Gmass = 0.0;
        }

        /*--------------------------------------------------------
         *  Calculate the total water available for E, Q, and Gw
         *  = sum( ice balance + Precip in )
         *--------------------------------------------------------*/
        massavailable = Gmass + MPglacial;

        /*-------------------------------------------------------
         *  Check if there is sufficient mass to grow a glacier
         *-------------------------------------------------------*/
        if (massavailable < 0.0) {
            fprintf(stderr, "HydroGlacial ERROR: year = %d, ep =%d \n", yr, ep);
            fprintf(stderr, " \t There is insufficient precipitation on the \n");
            fprintf(stderr, " \t glaciated area to grow the glacier at the \n");
            fprintf(stderr, " \t prescribed rate. \n");
            fprintf(stderr, " \t massavailable < 0.0 \n");
            fprintf(stderr, " \t massavailable = Gmass + MPglacial \n");
            fprintf(stderr, " \t massavailable \t = %e \n", massavailable);
            fprintf(stderr, " \t Gmass \t = %e (m^3) \n", Gmass);
            fprintf(stderr, " \t MPglacial  \t = %e (m^3) \n", MPglacial);
            fprintf(stderr, " \t Gmass = (lastarea-glacierarea)*fabs(lastela-ela) \n");
            fprintf(stderr, " \t lastarea   \t = %e (m^2) \n", lastarea);
            fprintf(stderr, " \t glacierarea\t = %e (m^2) \n", glacierarea);
            fprintf(stderr, " \t lastela    \t = %e  \n", lastela);
            fprintf(stderr, " \t ela        \t = %e  \n", ela);
            fprintf(stderr, " \t Parea       \t = %e  \n", Parea);
            fprintf(stderr, " \t ELAstart[ep]      \t = %e  \n", ELAstart[ep]);
            fprintf(stderr, " \t ELAchange[ep]      \t = %e  \n", ELAchange[ep]);
            fprintf(stderr, " \t setstartmeanQandQs  \t = %d \n", setstartmeanQandQs);
            exit(-1);
        }

        /*-----------------------------------------------------------------------
         *  Calculate the amount of ice lost to evaporation
         *  divide by the glaciated area, to get units of m of water equivelant
         *-----------------------------------------------------------------------*/
        Eiceannual = massavailable * dryevap[ep] / glacierarea;

        /*------------------------------------------------------------------
         *  Loop through the year and determine the melt days
         *  scale each event by the Temperature and Precip
         *  Later the scaled days will be assigned the appropriate
         *  amount of runoff
         *
         *  Also check to insure that we melt enough ice, and do not flood
         *  the basin.  Note that meltday will be scaled down further
         *  by the shoulder events.
         *
         *  The ranarray factor will add some randomness to the events
         *  ranarry has a mean of zero and std of 1
         *------------------------------------------------------------------*/
        meltflag = 1;
        maxmelt = 0.0;
        Tfix = 0.0;

        while (meltflag == 1) {
            totalmelt = 0.0;

            for (ii = 0; ii < daysiy; ii++) {
                Tcorrection = 0.0;

                if (Pdaily[ii] > 0.0) {
                    Tcorrection = 1.0;
                }

                if (Televday[glacierind][ii] + Tfix > 0.0) {
                    meltday[ii] = mx(Televday[glacierind][ii] + ranarray[nran] - Tcorrection + Tfix, 0.0);
                    nran++;
                    totalmelt += meltday[ii];
                    maxmelt = mx(meltday[ii], maxmelt);
                }
            }

            if (totalmelt == 0.0) {
                Tmean = 0.0;

                //          for (ii=daystrm[5]; ii<dayendm[7]; ii++ )
                for (ii = start_of(Jun); ii < end_of(Aug); ii++) {
                    Tmean += Televday[glacierind][ii];
                }

                //          Tmean/=(dayendm[7]-daystrm[5]);
                Tmean /= (end_of(Aug) - start_of(Jun));
                Tfix += mx(-Tmean, 1.0);
            } else {
                meltflag = 0;
            }
        }

        if (Tfix > 0.0) {
            fprintf(stderr, "\n HydroGlacial Warning: epoch = %d, year = %d \n", ep + 1, yr);
            fprintf(stderr, " \t The basin was too cold to melt enough glacial ice. \n");
            fprintf(stderr, " \t The daily temperatures used to melt ice were increased. \n");
            fprintf(stderr, " \t Tfix = %f (degC) \n", Tfix);
            fprintf(stderr, " \t Tmean = %f (degC) \n", Tmean);
            fprintf(fidlog, " HydroGlacial Warning: \n");
            fprintf(fidlog, " \t The basin was too cold to melt enough glacial ice. \n");
            fprintf(fidlog, " \t The daily temperatures used to melt ice were increased. \n");
            fprintf(fidlog, " \t Tfix = %f (degC) \n", Tfix);
            fprintf(fidlog, " \t Tmean = %f (degC) \n", Tmean);
        }

        /*---------------------------------------------------------------------------
         *  Create the shoulder events (Murray's version of flood wave attenuation)
         *  there is one left (preceeding) day scaled as:
         *   shoulderleft*event
         *  the main event is scaled down to:
         *   shouldermain*event
         *  there are 1 or more right (following days) scaled to:
         *   shoulderright[]*event
         *  1.0 = Sum(shoulderleft+shouldermain+shoulderright[])
         *---------------------------------------------------------------------------*/
        ii = 0;

        if (meltday[ii] > 0.0) {
            shldday[ii] += shoulderleft * meltday[ii];

            for (jj = 0; jj < shouldern - 2; jj++) {
                shldday[ii + jj + 1] += shoulderright[jj] * meltday[ii];
            }

            meltday[ii] = shouldermain * meltday[ii];
        }

        for (ii = 1; ii < daysiy; ii++) {
            Qice[ii - 1] = 0.0;
            Qice[ii] = 0.0;

            if (meltday[ii] > 0.0) {
                shldday[ii - 1] += shoulderleft * meltday[ii];

                for (jj = 0; jj < shouldern - 2; jj++) {
                    shldday[ii + jj + 1] += shoulderright[jj] * meltday[ii];
                }

                meltday[ii] = shouldermain * meltday[ii];
            }
        }

        /*-----------------------------------------------------------
         *    Add the shoulder events and the main events
         *    to get the total ice derived discharge.
         *    Also scale the discharges to match the actual ice melt.
         *    Convert to m^3/s
         *-----------------------------------------------------------*/
        for (ii = 0; ii < maxday - distbins[ELAindex]; ii++) {

            /*---------------------------------------------------------------------
             *  (Mark's version of routing)
             *  Add the time lag for the distance up the basin (distbins[elabin])
             *  Convert to m^3/s
             *---------------------------------------------------------------------*/
            dumint = distbins[ELAindex];
            Qice[ ii + dumint ] += (meltday[ii] + shldday[ii])
                * (massavailable - Eiceannual * glacierarea)
                / (totalmelt * dTOs);
        }

        /*---------------------------------------
         *  Add the carryover from the previous
         *  year and track it's mass
         *---------------------------------------*/
        Mwrap = 0.0;

        for (ii = 0; ii < maxday - daysiy; ii++) {
            Qice[ii] += Qicewrap[ii];
            Mwrap += Qicewrap[ii] * dTOs;
        }

        /*---------------------------------------------------------
         *  Add to the flux to the Groundwater pool
         *  Actual addition to the GW pool is done in HydroRain.c
         *---------------------------------------------------------*/
        for (ii = 0; ii < daysiy; ii++) {
            Qicetogw[ii] += percentgw[ep] * Qice[ii];
            Qice[ii]     -= Qicetogw[ii];
        }

        /*--------------------------
         *  Check the mass balance
         *--------------------------*/
        Mice = 0.0;
        Mgw  = 0.0;

        for (ii = 0; ii < maxday; ii++) {
            Mice += Qice[ii] * dTOs;
        }

        for (ii = 0; ii < daysiy; ii++) {
            Mgw  += Qicetogw[ii] * dTOs;
        }

        Mout = Mice + Mgw + Eiceannual * glacierarea;
        Minput = massavailable + Mwrap;

        if ((fabs(Mout - Minput) / Minput) > masscheck) {
            fprintf(stderr, "ERROR in HydroGlacial: \n");
            fprintf(stderr, "  Mass Balance error: Mout != Minput \n\n");
            fprintf(stderr, "\t fabs(Mout-Minput)/Minput > masscheck \n");
            fprintf(stderr, "\t note: masscheck set in HydroParams.h \n");
            fprintf(stderr, "\t masscheck = %f (%%) \n", masscheck);
            fprintf(stderr, "\t fabs(Mout-Minput)/Minput = %f (%%) \n\n",
                fabs(Mout - Minput) / Minput);
            fprintf(stderr, " \t Minput = massavailable + Mwrap \n");
            fprintf(stderr, " \t Minput \t\t = %e \n", Minput);
            fprintf(stderr, " \t massavailable \t = %e \n", massavailable);
            fprintf(stderr, " \t Mwrap \t\t = %e \n\n", Mwrap);
            fprintf(stderr, " \t Mout = Mice + Mgw + Eiceannual*glacierarea \n");
            fprintf(stderr, " \t Mout \t\t = %e \n", Mout);
            fprintf(stderr, " \t Mice \t\t = %e \n", Mice);
            fprintf(stderr, " \t Mgw \t\t = %e \n", Mgw);
            fprintf(stderr, " \t Eiceannual \t = %e \n\n", Eiceannual * glacierarea);
            exit(-1);
        }

    }   /* endif glacial */

#if DBG
    fprintf(fidlog, "\n HydroGlacial: \t year = %d \n\n", yr);

    fprintf(fidlog, " \t Mass Wrap \t = %e (m^3) \n\n", Mwrap);

    fprintf(fidlog, " \t ela \t\t = %f (m) \n", ela);
    fprintf(fidlog, " \t elabin \t = %f (m) \n", elabin);
    fprintf(fidlog, " \t elevbinsize \t = %f (m) \n\n", elevbinsize);

    fprintf(fidlog, " \t maxalt[ep] \t = %f (m) \n", maxalt[ep]);
    fprintf(fidlog, " \t glacierelev \t = %f (m) \n\n", glacierelev);

    fprintf(fidlog, " \t smallg \t = %e (m^2) \n", smallg);
    fprintf(fidlog, " \t bigg \t\t = %e (m^2) \n", bigg);
    fprintf(fidlog, " \t glacierarea \t = %e (m^2) \n", glacierarea);
    fprintf(fidlog, " \t approxarea \t = %e (m^2) \n\n", approxarea);

    fprintf(fidlog, " \t ela \t\t = %f (m) \n", ela);
    fprintf(fidlog, " \t lastela \t = %f (m) \n", lastela);
    fprintf(fidlog, " \t diff(ela) \t = %f (m) \n\n", fabs(lastela - ela));

    fprintf(fidlog, " \t lastarea \t = %e (m^2) \n", lastarea);
    fprintf(fidlog, " \t glacierarea \t = %e (m^2) \n", glacierarea);
    fprintf(fidlog, " \t diff(area) \t = %e (m^2) \n\n", lastarea - glacierarea);

    fprintf(fidlog, " \t MPglacial \t = %e (m^3) \n", MPglacial);
    fprintf(fidlog, " \t GMass \t\t = %e (m^3) \n", Gmass);
    fprintf(fidlog, " \t Mass available\t = %e (m^3) \n\n", massavailable);

    fprintf(fidlog, " \t Mout \t\t = %e \n", Mout);
    fprintf(fidlog, " \t Mout = Mice + Mgw + Eiceannual*glacierarea \n");
    fprintf(fidlog, " \t Mice \t\t = %e \n", Mice);
    fprintf(fidlog, " \t Mgw \t\t = %e \n", Mgw);
    fprintf(fidlog, " \t Eiceannual \t = %e \n", Eiceannual * glacierarea);

    /*----------------------------------------------------------------------------
     *  Print out glacial Q for error checking.
     *  The % at the begining of each text line is for a comment line in matlab.
     *  This enables the file to be read directly into matlab for plotting.
     *----------------------------------------------------------------------------*/
    if (tblstart[ep] <= yr && yr <= tblend[ep]) {
        if ((fid = fopen("hydro.ice", "a+")) == NULL) {
            printf("  HydroGlacial ERROR: Unable to open the Qice file hydro.ice \n");
            printf("     non-fatal error, continueing. \n\n");
        } else {
            fprintf(fid, "%%\n%%\n%% HydroGlacial output: \n%%\n");
            fprintf(fid, "%%Daily predicted Qice for epoch %d \n%%\n", ep + 1);
            fprintf(fid, "%%Year \t Day \t Qice(m^3/s) \t Qicetogw(m^3/s) \n");
            fprintf(fid, "%%---- \t --- \t ----------- \t ------------ \n");

            for (ii = 0; ii < daysiy; ii++) {
                fprintf(fid, "%d \t %d \t %f \t %f \n", yr, ii + 1, Qice[ii], Qicetogw[ii]);
            }

            for (ii = daysiy; ii < maxday; ii++) {
                fprintf(fid, "%d \t %d \t %f \t NaN \n", yr, ii + 1, Qice[ii]);
            }

            fclose(fid);
        }
    }

#endif

    return (err);
}  /* end of HydroGlacial */
