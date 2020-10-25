/*
 * HydroSetGeoParams.c
 *
 * The subroutine sets the constant parameters of Qsbar based on the
 * geographic position of the river mouth and average basin temperature,
 * according to a paper of Syvitski, J.P.M., Peckham, S.D., Hilberman, R.D.
 * and Mulder, T., (in press).
 * Predicting the terrestrial flux of sediment to the global
 * ocean: A planetary perspective. Marine Geology.
 *
 *  Author:    A.J. Kettner  (August 2002)(February 2003)(november 2003)
 *
 *
 * Qsbar = alpha3 * pow(A,alpha4) * pow(H,alpha5) * exp(k * Tbar)
 * Alpha3, alpha4 and alpha5 and k are set here.
 * Tbar  = Tmean - ((lapserate[ep] * maxalt[ep])/3.0);
 * Tmean = (Tstart[ep] + (Tstart[ep] + (Tchange[ep]*nyears[ep])))/2;
 *
 *--------------------------------------------------------------------
 *|Global Sector                  | alpha3 | alpha4 | alpha5 |  k    |
 *|                               |        |        |        |       |
 *|Polar (T<0 deg.C)              | 2e-5   | 0.50   | 1.50   |  0.1  |
 *|Temperate N (Lat>30N, T>0deg.C)| 6.1e-5 | 0.55   | 1.12   |  0.07 |
 *|Tropics N (Lat 0 to 30N)       | 0.31   | 0.40   | 0.66   | -0.1  |
 *|Tropics S (Lat 0 to 30S)       | 0.57   | 0.50   | 0.37   | -0.1  |
 *|Temperate S (Lat>30S, T>0deg.C)| 1.3e-3 | 0.43   | 0.96   |  0.0  |
 *--------------------------------------------------------------------
 *
 * Or:
 * Qsbar = alpha6 * pow(Q,alpha7) * pow(H,alpha8) * exp(k*Tbar)
 * Alpha6, alpha7, alpha8 and k are set here.
 *
 *--------------------------------------------------------------------
 *|Global Sector                  | alpha6 | alpha7 | alpha8 |  k    |
 *|                               |        |        |        |       |
 *|Polar (T<0 deg.C)              | 1.3e-4 | 0.55   | 1.50   |  0.1  |
 *|Temperate N (Lat>30N, T>0deg.C)| 1.1e-3 | 0.53   | 1.10   |  0.06 |
 *|Tropics N (Lat 0 to 30N)       | 2.0    | 0.45   | 0.57   | -0.09 |
 *|Tropics S (Lat 0 to 30S)       | 162    | 0.65   |-0.05   | -0.16 |
 *|Temperate S (Lat>30S, T>0deg.C)| 1.1e-3 | 0.53   | 1.10   |  0.06 |
 *--------------------------------------------------------------------
 * NOTE THAT TEMPERATE S GOT THE SAME VALUES AS THE TEMPERATE N. THIS BECAUSE
 * NO VALUES WHERE IN THE PAPER FOR TEMPERATE S. FOR NOW, THE VALUES FOR TEMPERATE
 * N ARE USED.
 *
 *  Variable    Def.Location        Type    Units   Usage
 *  --------    ------------        ----    -----   -----
 *  changeyear  HydroSetGeoParams.c double  years   The year in which the Temp switches
 *  err         various             int     -       error flag, halts program
 *  i           various             int     -       counter
 *  Tbar        HydroSetGeoParams.c double  deg.C   Mean temp. for the whole basin
 *  Tbarend     HydroSetGeoParams.c double  deg.C   Temp at the last year of the run corrected for the whole basin
 *  Tbarstart   HydroSetGeoParams.c double  deg.C   Temp at the first year of the run
 *  Tdummy      HydroSetGeoParams.c double  deg.C   Total temp used for real data
 *  Tend        HydroSetGeoParams.c double  deg.C   Temp at the last year of the run
 *  Tmean       HydroSetGeoParams.c double  deg.C   Mean temp. not corrected for the whole basin yet.
 *
 */

#include "hydroparams.h"
#include "hydroclimate.h"
#include "hydroreadclimate.h"
#include "hydroinout.h"

/*------------------------------
 *  Start of HydroSetGeoParams
 *------------------------------*/
int
hydrosetgeoparams(gw_rainfall_etc* gw_rain)
{
    int err, i;
    double Tend, Tmean, Tbar, Tdummy; /*, Tbarstart, Tbarend, changeyears;*/

    /*----------------------------------------------------------------
     * Set all the constants of the Qsbar
     * (Qsbar = alpha3 * pow(A,alpha4) * pow(H,alpha5) * exp(k * Tbar)
     *----------------------------------------------------------------*/
    err = 0;

    if (raindatafile == 1) {
        Tdummy = 0.0;

        for (i = 0; i < nyears[ep]; i++) {
            Tdummy += gw_rain->Tperyear[i];
        }

        Tmean = Tdummy / nyears[ep];
    }

    else {
        Tend        = Tstart[ep] + (Tchange[ep] * nyears[ep]);
        Tmean       = (Tstart[ep] + Tend) / 2;
    }

    Tbar = Tmean - ((lapserate[ep] * maxalt[ep]) / 3.0);

    if (lat > 30 && Tbar > 0) {                               /* Temperate North */
        alpha3 = 0.000061;
        alpha4 = 0.55;
        alpha5 =  1.12;
        k1 = 0.07;
    }

    if ((lat > 0 || lat == 0) && lat < 30 && Tbar > 0) {      /* Tropics North */
        alpha3 = 0.31;
        alpha4 = 0.40;
        alpha5 =  0.66;
        k1 = -0.1;
    }

    if ((lat > -30 || lat == -30) && lat < 0 && Tbar > 0) {   /* Tropics South */
        alpha3 = 0.57;
        alpha4 = 0.50;
        alpha5 =  0.37;
        k1 = -0.1;
    }

    if (lat < -30 && Tbar > 0) {                              /* Temperate South */
        alpha3 = 0.0013;
        alpha4 = 0.43;
        alpha5 =  0.96;
        k1 = 0.0;
    }

    if (Tbar <= 0.0) {                         /* Polar; South or North */
        alpha3 = 0.00002;
        alpha4 = 0.50;
        alpha5 =  1.50;
        k1 = 0.1;
    }

    if (lat > 30 && Tbar > 0) {                               /* Temperate North */
        alpha6 = 0.0011;
        alpha7 = 0.53;
        alpha8 =  1.1;
        k2 = 0.06;
    }

    if ((lat >= 0.0) && lat < 30 && Tbar > 0) {      /* Tropics North */
        alpha6 = 2.0;
        alpha7 = 0.45;
        alpha8 =  0.57;
        k2 = -0.09;
    }

    if ((lat > -30 || lat == -30) && lat < 0 && Tbar > 0) {   /* Tropics South */
        alpha6 = 162;
        alpha7 = 0.65;
        alpha8 =  -0.05;
        k2 = -0.16;
    }

    if (lat < -30 && Tbar > 0) {                              /* Temperate South */
        alpha6 = 0.0011;
        alpha7 = 0.53;
        alpha8 =  1.1;
        k2 = 0.06;
    }

    if (Tbar <= 0.0) {                         /* Polar; South or North */
        alpha6 = 0.00013;
        alpha7 = 0.55;
        alpha8 =  1.50;
        k2 = 0.1;
    }

    return (err);
} /* end of HydroSetGeoParams.c */

