/*
 * HydroRain.c      Calculates the daily rain fall and rain derived runoff
 *          for each altitude bin.  Also calculates the rain
 *          derived groundwater flow, evaporation and time lag.
 *
 *  Physically based precipitation/groundwater/evaporation routine based on:
 *  M. Sivapalan, J.K. Ruprecht, N.R. Viney, 1996.  Water and Salt Balance
 *      Modelling to predict the effects of Land-Use changes in forested
 *      catchments. 1. Small Catchment water balance model.
 *      Hydrological Processes, V.10, P.393-411.
 *
 *  Original:   M.D. Morehead   July 1996   precip.F
 *  1st C Version:  M.D. Morehead   1998
 *      Author2:        A.J. Kettner    October 2002
 *
 * Variable     Def.Location    Type    Units   Usage
 * --------     ------------    ----    -----   -----
 * Mannualin    HydroRain.c     double  m^3/a   annual mass input to rain routine
 * Mannualout   HydroRain.c     double  m^3/a   annual mass output from rain routine
 * Minput       HydroRain.c     double  m^3     daily mass input to rain routine
 * Mout         HydroRain.c     double  m^3     daily mass input to rain routine
 * Mwrap        HydroRain.c     double  m^3     mass input to rain routine from previous year
 * Pground      HydroRain.c     double  m/day   ground precipitation
 * Qi1, Qi2     HydroRain.c     double  m^3/s   temporary variables for Qinfiltration
 * Qinfiltexcess HydroRain.c    double  m^3/s   river discharge from infiltration excess
 * Qinfiltration HydroRain.c    double  m^3/s   = Pground-Qrain[ii]
 * Qrainlag     HydroRain.c     double  m^3/s   temporary array for routing
 * Qsaturationexcess HydroRain.c double m^3/s   river discharge from saturation excess
 * Qsslag       HydroRain.c     double  m^3/s   temporary array for routing
 * Qtogw        HydroRain.c     double  m^3/s   Rain derived discharge to the GW pool
 * err          various         int     -       error flag, halts program
 * ii           various         int     -       temporary loop counter
 * infiltrate   HydroRain.c     double  m/day   infiltration capacity
 * jj           various         int     -       temporary loop counter
 * kk           various         int     -       temporary loop counter
 * ratio1       HydroRain.c     double  -       ratio of the fullness of the GW pool
 * ratio2       HydroRain.c     double  -       linear interpolation for infiltration
 * shldday[]    HydroRain.c     double  m^3/s   shoulder discharge array
 *
 */

#include <stdlib.h>

#include "hydroclimate.h"
#include "hydroparams.h"
#include "hydrotimeser.h"

#ifdef DBG
    #include "hydroinout.h"
#endif

/*----------------------
 *  Start of HydroRain
 *----------------------*/
int
hydrorain()
{

    /*-------------------
     *  Local Variables
     *-------------------*/
#ifdef DBG
    FILE* fid;
#endif

    double infiltrate, Pground[daysiy];
    double Qsaturationexcess[daysiy], Qinfiltexcess[daysiy];
    double Qinfiltration[daysiy], Qi1, Qi2, Qtogw, shldday[maxday];
    double  Mout, Mannualin, Mannualout;
    double Qrainlag[maxday], Qsslag[maxday];
    double ratio1, ratio2;
    int ii, jj, kk, err;

    /*------------------------
     *  Initialize Variables
     *------------------------*/
    err = 0;
    MPrain = 0.0;
    Mannualin = 0.0;
    Mannualout = 0.0;

    for (ii = 0; ii < maxday; ii++) {
        Qrainlag[ii] = 0.0;
        Qsslag[ii]   = 0.0;
        shldday[ii]  = 0.0;
    }

    /*-------------------------
     *  Loop through the year
     *-------------------------*/
    for (ii = 0; ii < daysiy; ii++) {

        /*----------------------------------------------------------------------
         *  Find the area being rained on
         *  Temperature must be above freezing
         *    All rain (P with T>=0.0) is handled here.
         *    All snow/ice (P with T<0.0) is handled in HydroSnow/HydroGlacier
         *----------------------------------------------------------------------*/
        rainarea[ii] = 0.0;

        for (kk = 0; kk < nelevbins && kk < FLAindex[ii]; kk++) {
            rainarea[ii] += areabins[kk];
        }

        /*--------------------------------------------------------------
         *  Get the remaining GW pool from the previous day
         *  the first day of the year is taken care of in HydroTrend.c
         *--------------------------------------------------------------*/
        if (ii > 0) {
            gwstore[ii] = gwstore[ii - 1];
        }

        /*---------------------------------------------------------------------------
         *  Calculate groundwater discharge (subsurface storm flow) and evaporation
         *  Qss (m^3/s), Egw (m/day), gwstore (m^3)
         *  Only drain down to gwmin
         *
         *  Having this first builds in a one delay lag to the GW discharge
         *  This also helps prevent the Glacier and Snow from totally filling
         *  the pool, and leaves some room for Rain.
         *
         *  The lines E=max(E,0.0) and Qss=max(Qss,0.0) prevent rare errors due
         *  to very small numbers when gwstore is close to gwmin.
         *---------------------------------------------------------------------------*/
        Egw[ii] = mn(alphagwe[ep] * pow((gwstore[ii] - gwmin[ep]) / (gwmax[ep] - gwmin[ep]),
                    betagwe[ep]), (gwstore[ii] - gwmin[ep]) / totalarea[ep]);
        Egw[ii] = mx(Egw[ii], 0.0);
        gwstore[ii] -= Egw[ii] * totalarea[ep];

        Qss[ii] = mn(alphass[ep] * pow((gwstore[ii] - gwmin[ep]) / (gwmax[ep] - gwmin[ep]),
                    betass[ep]), (gwstore[ii] - gwmin[ep]) / dTOs);
        Qss[ii] = mx(Qss[ii], 0.0);
        gwstore[ii] -= Qss[ii] * dTOs;

        /*-------------------------------------------------------------------------------
         *  Add the other sources to the Groundwater pool
         *  allocate overflow to Qexceedgw and add to river discharge in HydroSumFlow.c
         *-------------------------------------------------------------------------------*/
        if (gwstore[ii] < gwmax[ep]) {
            Qtogw = mn(Qicetogw[ii] + Qnivaltogw[ii], (gwmax[ep] - gwstore[ii]) / dTOs);
        } else {
            Qtogw = 0.0;
        }

        gwstore[ii]  += Qtogw * dTOs;
        Qexceedgw[ii] = Qicetogw[ii] + Qnivaltogw[ii] - Qtogw;

        /*--------------------------------------
         *  Calculate commonly used quantities
         *--------------------------------------*/
        ratio1  = (gwstore[ii] - gwmin[ep]) / (gwmax[ep] - gwmin[ep]);
        ratio2  = (Ko[ep] - pcr[ep]) / (pmax[ep] - pcr[ep]);

        /*------------------------------------------------------------
         *  Calculate the Ground Precipitation (m/day)
         *  the remainder is evaporated from canopy interception
         *  keep track of evaporation (m/day) for mass balance check
         *------------------------------------------------------------*/
        Pground[ii] = mx(alphag[ep] + betag[ep] * Pdaily[ii], 0.0);
        Ecanopy[ii] = Pdaily[ii] - Pground[ii];

        /*----------------------------------------------------
         *  Calculate Surface Runoff (m^3/s)
         *  Sum of saturation excess and infiltration excess
         *  remainder goes to groundwater pool
         *----------------------------------------------------*/

        /*-----------------------------------------------------
         *  Calculate the Saturation Excess
         *  this is all the rain that falls on saturated soil
         *  and cannot go into the groundwater pool
         *-----------------------------------------------------*/
        if (gwstore[ii] > gwmin[ep]) {
            Qsaturationexcess[ii] = (Pground[ii] * rainarea[ii] / dTOs) * alphac[ep] * pow(ratio1,
                    betac[ep]);
        } else {
            Qsaturationexcess[ii] = 0.0;
        }

        /*--------------------------------------------------------------------
         *  Calculate Theoretical Infiltration Rate based on Precip  (m/day)
         *    a) below pcr, 100% infiltration
         *    b) above pmax, max hydr. cond.
         *    c) linear fit in between
         *--------------------------------------------------------------------*/
        if (Pground[ii] < pcr[ep]) {
            infiltrate = Pground[ii];
        } else if (Pground[ii] > pmax[ep]) {
            infiltrate = Ko[ep];
        } else {
            infiltrate = Pground[ii] * ratio2 + pcr[ep] * (1 - ratio2);
        }

        /*-------------------------------------------------------------------------
         *  Calculate infiltration "discharge" to the groundwater pool (m^3/s)
         *  1st) calculate the theoretical infiltration discharge
         *  2nd) calculate the discharge that fills the groundwater pool in a day
         *  take the minimum so as not to overfill the pool
         *-------------------------------------------------------------------------*/
        if (gwstore[ii] > gwmin[ep]) {
            Qi1 = infiltrate * (1.0 - ratio1) * rainarea[ii] / dTOs;
        } else {
            Qi1 = infiltrate * rainarea[ii] / dTOs;
        }

        Qi2 = (gwmax[ep] - gwstore[ii]) / dTOs;

        Qinfiltration[ii] = mn(Qi1, Qi2);
        gwstore[ii] += Qinfiltration[ii] * dTOs;

        /*--------------------------------------------------
         *  Calculate the infiltration excess (m^3/s)
         *  this is the runoff that:
         *    a) is not due to saturation excess
         *    b) and can not get into the groundwater pool
         *--------------------------------------------------*/
        Qinfiltexcess[ii] = mx(0.0, \
                Pground[ii] * rainarea[ii] / dTOs - Qsaturationexcess[ii] - Qinfiltration[ii]);

        /*-------------------------------------------------------
         *  Calculate Surface Runoff (m^3/s)
         *    Sum of saturation excess and infiltration excess
         *    remainder went to groundwater pool or evaporation
         *-------------------------------------------------------*/
        Qrain[ii] = Qsaturationexcess[ii] + Qinfiltexcess[ii];

        /*---------------------------------------------------------------------------
         *  Add the carryover from the previous year
         *
         *  NOTE: adding at this stage assumes the wrapped rain is due to the wave
         *  propagating down the river and that this water does not have additional
         *  contributions to the Evap or gw pools
         *---------------------------------------------------------------------------*/

        if (ii < wrapday) {
            Qrain[ii] += Qrainwrap[ii];
        }

        /*------------------------------------------------
         *  Add to the daily rain mass balance (m^3/day)
         *------------------------------------------------*/
        Minput = Pdaily[ii] * rainarea[ii];

        if (ii < maxday - daysiy) {
            Minput += Qrainwrap[ii] * dTOs;
        }

        Mout = Ecanopy[ii] * rainarea[ii] + (Qrain[ii] + Qinfiltration[ii]) * dTOs;

        /*-----------------------------------------------
         *  Add to the annual rain mass balance (m^3/a)
         *-----------------------------------------------*/
        MPrain   += Pdaily[ii] * rainarea[ii];
        Mannualin    += Minput;
        Mannualout   += Mout;

        /*---------------------------------------------
         *  Check the daily rain mass balance (m^3/d)
         *---------------------------------------------*/
        if (Minput > 0. && fabs(Mout - Minput) / Minput > masscheck) {
            fprintf(stderr, "ERROR: in HydroRain.c: \n");
            fprintf(stderr, "   Daily Mass Balance Error: Mout != Minput \n\n");
            fprintf(stderr, "   fabs(Mout-Minput)/Minput > masscheck \n");
            fprintf(stderr, "   note: masscheck set in HydroParams.h \n");
            fprintf(stderr, "   masscheck = %e (m^3) \n", masscheck);
            fprintf(stderr, "   fabs(Mout-Minput)/Minput = %e (m^3) \n\n",
                fabs(Mout - Minput) / Minput);
            fprintf(stderr, " \t Minput = Pdaily + Qrainwrap \n");
            fprintf(stderr, " \t Minput \t\t = %e (m^3) \n", Minput);
            fprintf(stderr, " \t Pdaily[ii]*rainarea[ii] \t = %e (m^3) \n",
                Pdaily[ii]*rainarea[ii]);

            if (ii < maxday - daysiy) {
                fprintf(stderr, " \t Qrainwrap[ii] \t\t = %e (m^3) \n\n", Qrainwrap[ii]*dTOs);
            } else {
                fprintf(stderr, " \t Qrainwrap[ii] \t\t = 0.0 (m^3) \n\n");
            }

            fprintf(stderr, " \t Mout = Ecanopy + Qrain + Qinfiltration \n");
            fprintf(stderr, " \t Mout \t\t = %e (m^3) \n", Mout);
            fprintf(stderr, " \t Ecanopy[ii]*rainarea[ii] \t\t = %e (m^3) \n",
                Ecanopy[ii]*rainarea[ii]);
            fprintf(stderr, " \t Qrain[ii]*dTOs \t\t = %e (m^3) \n", Qrain[ii]*dTOs);
            fprintf(stderr, " \t Qinfiltration[ii]*dTOs \t = %e (m^3) \n\n",
                Qinfiltration[ii]*dTOs);
            fprintf(stderr, " \t Day \t d \t %d \n", ii + 1);
            fprintf(stderr, " \t Year \t d \t %d \n", yr + 1);
            fprintf(stderr, " \t Epoch \t - \t %d \n", ep + 1);
            exit(-1);
        }

        /*------------------------------------------------
         *  Check for over/under-filled groundwater pool
         *  return error
         *------------------------------------------------*/
        if (gwstore[ii] < gwmin[ep] / 1.001 || gwstore[ii] > gwmax[ep] * 1.001) {
            fprintf(stderr, "WARNING in HydroRain.c \n");
            fprintf(stderr, "   gwstore out of range. \n");
            fprintf(stderr, "   criteria:  gwmin < gwstore < gwmax \n");
            fprintf(stderr, "      gwmax   (m^3) = %e \n", gwmax[ep]);
            fprintf(stderr, "      gwstore (m^3) = %e \n", gwstore[ii]);
            fprintf(stderr, "      gwmin   (m^3) = %e \n", gwmin[ep]);
            fprintf(stderr, "      Qss[ii] (m^3) = %e \n", Qss[ii]*dTOs);
            fprintf(stderr, "      Egw[ii] (m^3) = %e \n", Egw[ii]*totalarea[ep]);
            err = 1;
        }
    }   /* end day loop */

    /*------------------------------------------------------------
     *  Use shoulder on Rain events if the basin is large enough
     *------------------------------------------------------------*/
    if ((int)(basinlength[ep] / (avgvel[ep]*dTOs)) >= 3 || Rvol[ep] > 0.0)  {

        /*------------------------------------------------------------
         *  Create the shoulder events (Murray's version of routing)
         *    there is one left (preceeding) day scaled as:
         *      shoulderleft*event
         *    the main event is scaled down to:
         *      shouldermain*event
         *    there are 1 or more right (following days) scaled to:
         *      shoulderright[]*event
         *
         *    1.0 = Sum(shoulderleft+shouldermain+shoulderright[])
         *------------------------------------------------------------*/
        ii = 0;

        if (Qrain[ii] > 0.0) {
            shldday[ii] += shoulderleft * Qrain[ii];

            for (jj = 0; jj < shouldern - 2; jj++) {
                shldday[ii + jj + 1] += shoulderright[jj] * Qrain[ii];
            }

            Qrain[ii] = shouldermain * Qrain[ii];
        }

        for (ii = 1; ii < daysiy; ii++) {
            if (Qrain[ii] > 0.0) {
                shldday[ii - 1] += shoulderleft * Qrain[ii];

                for (jj = 0; jj < shouldern - 2; jj++) {
                    shldday[ii + jj + 1] += shoulderright[jj] * Qrain[ii];
                }

                Qrain[ii] = shouldermain * Qrain[ii];
            }
        }

        /*----------------------------------------------------------------
         *  Add the shoulder events and the main events to get the total
         *  rain derived discharge
         *----------------------------------------------------------------*/
        for (ii = 0; ii < maxday; ii++) {
            Qrain[ii] += shldday[ii];
        }
    } /* endif shoulder */

    /*---------------------------------------------------------------------
     *  (Mark's version of routing)
     *  Remove any lagged discharge from the initial rain discharge event
     *---------------------------------------------------------------------*/
    for (ii = 0; ii < daysiy; ii++) {
        for (kk = 0; kk < nelevbins && kk < FLAindex[ii]; kk++) {
            Qrainlag[ii + distbins[kk]] += Qrain[ii] * areabins[kk] / rainarea[ii];
            Qsslag[ii + distbins[kk]]   +=   Qss[ii] * areabins[kk] / rainarea[ii];
            Qrain[ii] -= Qrain[ii] * areabins[kk] / rainarea[ii];
            Qss[ii]   -=   Qss[ii] * areabins[kk] / rainarea[ii];
        }
    }

    /*------------------------------------
     *  Add the lagged discharge back in
     *------------------------------------*/
    for (ii = 0; ii < maxday; ii++) {
        Qrain[ii] += Qrainlag[ii];
        Qss[ii]   += Qsslag[ii];
    }

    /*--------------------------------
     *  Add the GW wrapped discharge
     *--------------------------------*/
    for (ii = 0; ii < wrapday; ii++) {
        Qss[ii]   += Qsswrap[ii];
    }

    /*---------------------------------------
     *  Check the annual mass balance (m^3)
     *---------------------------------------*/
    if (fabs(Mannualout - Mannualin) / Mannualin > masscheck) {
        fprintf(stderr, "ERROR: in HydroRain.c: \n");
        fprintf(stderr, "   Annual Mass Balance Error: Mout != Minput \n\n");

        fprintf(stderr, "   fabs(Mannualout-Mannualin)/Mannualin > masscheck \n");
        fprintf(stderr, "   note: masscheck set in HydroParams.h \n");
        fprintf(stderr, "   masscheck = %e (m^3) \n", masscheck);
        fprintf(stderr, "   fabs(Mannualout-Mannualin)/Mannualin = %e (m^3) \n\n",
            fabs(Mannualout - Mannualin) / Mannualin);

        fprintf(stderr, " \t Mannualin = Sum(Pdaily+Qrainwrap) \t\t = %e (m^3) \n", Mannualin);
        fprintf(stderr, " \t Mannualout = Sum(Ecanopy+Qrain+Qinfiltration) \t = %e (m^3) \n\n",
            Mannualout);

        fprintf(stderr, " \t Year \t d \t %d \n", yr + 1);
        fprintf(stderr, " \t Epoch \t - \t %d \n\n", ep + 1);

        exit(-1);
    }

#ifdef DBG
    /*--------------------------------------------
     *  print out summary stats for the log file
     *--------------------------------------------*/
    fprintf(fidlog, "\n HydroRain Summary values: \n\n");
    fprintf(fidlog, " \t Mannualin = Sum(Pdaily+Qrainwrap) \t\t = %e (m^3) \n", Mannualin);
    fprintf(fidlog, " \t Mannualout = Sum(Ecanopy+Qrain+Qinfiltration) \t = %e (m^3) \n\n",
        Mannualout);

    /*---------------------------------------------------------------------------
     *  print out precipitation for error checking
     *  the % at the begining of each text line is for a comment line in matlab
     *  this enables the file to be read directly into matlab for plotting
     *---------------------------------------------------------------------------*/
    if (tblstart[ep] <= yr && yr <= tblend[ep]) {
        if ((fid = fopen("hydro.p1", "a+")) == NULL) {
            printf("  HydroRain ERROR: Unable to open the precipitation file hydro.p1 \n");
            printf("     non-fatal error, continueing. \n\n");
        } else {
            fprintf(fid, "%%\n%%\n%% HydroRain output: \n%%\n");
            fprintf(fid, "%%Daily predicted precipitation and rain runoff for epoch %d \n%%\n",
                ep + 1);
            fprintf(fid,
                "%%Year \t Day \t Qrain(m^3/s) \t Qsat(m^3/s) \t Qine(m^3/s) \t Qinf(m^3/s) \t Qss(m^3/s) \n");
            fprintf(fid,
                "%%---- \t --- \t ------------ \t ------------ \t ----------- \t ----------- \t ----------- \n");

            for (ii = 0; ii < daysiy; ii++)
                fprintf(fid, "%d \t %d \t %f \t %f \t %f \t %f \t %f \n", yr, ii + 1, \
                    Qrain[ii], Qsaturationexcess[ii], Qinfiltexcess[ii], Qinfiltration[ii], Qss[ii]);

            fclose(fid);
        }
    }

    if (tblstart[ep] <= yr && yr <= tblend[ep]) {
        if ((fid = fopen("hydro.p2", "a+")) == NULL) {
            printf("  HydroRain ERROR: Unable to open the precipitation file hydro.p2 \n");
            printf("     non-fatal error, continueing. \n\n");
        } else {
            fprintf(fid, "%%\n%%\n%% HydroRain output: \n%%\n");
            fprintf(fid, "%%Daily predicted precipitation and rain runoff for epoch %d \n%%\n",
                ep + 1);
            fprintf(fid,
                "%%Year \t Day \t Pg(m^3/s) \t Ecanopy(m^3/s)\t Egw(m^3/s) \t GWstore(m^3/s) \n");
            fprintf(fid,
                "%%---- \t --- \t ------------- \t ------------ \t ------------  \t ------------\n");

            for (ii = 0; ii < daysiy; ii++)
                fprintf(fid, "%d \t %d \t %f \t %f \t %f \t %f \n", yr, ii + 1, \
                    Pground[ii]*rainarea[ii] / dTOs, Ecanopy[ii]*rainarea[ii] / dTOs,
                    Egw[ii]*totalarea[ep] / dTOs, \
                    gwstore[ii] / dTOs);

            fclose(fid);
        }
    }

#endif

    return (err);
}   /* end of HydroRain.c */
