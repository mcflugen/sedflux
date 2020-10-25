/*
 *  HydroSetParams.c
 *
 *  Author1:    S.D. Peckman (december 2001)
 *  Author2:    A.J. Kettner (september 2002)
 *
 *
 *  In hydroreadinput.c, units are converted as follows:
 *  Multiply totalarea by 1e6.  (km^2 -> m^2)
 *  Multiply basinlength by 1000.  (km -> m)
 *  Divide Ko by 1000.  (mm/day -> m/day)
 *  Divide Lapse rate by 1000.  (mm/day -> m/day)
 *  Some others are done by this routine.
 *
 */

#include "hydroparams.h"
#include "hydroclimate.h"

/*---------------------------
 *  Start of HydroSetParams
 *---------------------------*/
int
hydrosetparams()
{

    int ii, err;

    /*----------------------------------------
     *  Hardwire parameters for complete run
     *----------------------------------------*/
    err = 0;
    webflag = 0;            /*0 = weboption is off. In HydroTrend.c, this option can be turned on */

    /*---------------------------------------
     *  Hardwired Parameters for all Epochs
     *---------------------------------------*/
    for (ii = 0; ii < maxepoch; ii++) {
        rhowater[ii]     = rho_water;    /* 1000 */
        rhosed[ii]       = rho_sed;      /* 2670 */
        alphac[ii]       = 0.98;    /* saturation excess coeff */
        betac[ii]        = 1.00;    /* saturation excess exponent */
        alphag[ii]       = -0.0001;    /* groundwater precip offset (m/day) */
        betag[ii]        = 0.85;    /* groundwater precip slope */
        alphagwe[ii]     = 0.0020;     /* groundwater evap coeff (m/day)*/
        betagwe[ii]      = 1.0;     /* groundwater evap exponent */
        pmax[ii]         = 0.400;   /* precip need to reach max cond. (m/day) */
        Meltrate[ii]     = 0.003;   /* (m/degC) */
        percentgw[ii]    = 0.15;    /* percent of nival&ice as groundwater */
        pcr[ii]          = 0.010;     /* crit. precip for infilt. excess (m/day) */
        bethaexpo        = 1.38;    /* volume-surface area exponent glaciers */
        bethaglacier     = 31.11716; /* volume-surface area multiplier glaciers */
        /* Meltrate[ii]  = 0.002;   (m/degC) */
        /* percentgw[ii] = 0.06;    percent of nival&ice as groundwater */
        /* pcr[ii]        = 2.0;    crit. precip for infilt. excess (mm/day) */

        /* widcof[ii]    = ???;   */
        /* widpow[ii]   = 0.5;   */
        /* velcof[ii]   = ???;   */
        /* velpow[ii]   = 0.1;   */
    }

    return (err);
}  /* end of HydroSetParams */
