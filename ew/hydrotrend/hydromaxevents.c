/*
 *  HydroMaxEvent.c
 *
 *  Calculates the highest discharge events (Qpeaks)
 *  On those events the number of outlets are recalculated.
 *
 *
 *  Author:    A.J. Kettner     (March 2003)
 *
 *  Variable            Def.Location    Type    Units   Usage
 *  --------            ------------    ----    -----   -----
 *  err                 various         int     -       error flag, halts program
 *  x                   hydromaxevent.c int     -       temporary loop counter
 *  y                   hydromaxevent.c int     -       temporary loop counter
 *  Qpeakalleventstemp  hydromaxevent.c double  -       temporary variable to store Qpeakvalue
 *
 */

#include "hydroclimate.h"
#include "hydroparams.h"
#include "hydrotimeser.h"

/*--------------------------
 *  Start of HydroMaxEvent
 *--------------------------*/
int
hydromaxevents()
{

    /*-------------------
     *  Local Variables
     *-------------------*/
    int err, p, x, y;
    double Qpeakalleventstemp, Qpeakalleventstemp2;
    err = 0;

    /*----------------------
     *  Setting the events
     *----------------------*/
    if (yr == syear[ep])
        for (p = 0; p < eventsnr[ep]; p++) {
            Qpeakallevents[ep][p] = Qpeakevents[p];
        } else
        for (p = eventsnr[ep] - 1; p >= 0; p--)
            for (x = 0; x < eventsnr[ep]; x++) {
                if (Qpeakevents[p] > Qpeakallevents[ep][p - x]) {
                    Qpeakalleventstemp = Qpeakallevents[ep][p - x];
                    Qpeakallevents[ep][p - x] = Qpeakevents[p];

                    for (y = p - x - 1; y >= 0; y--) {
                        Qpeakalleventstemp2 = Qpeakallevents[ep][y];
                        Qpeakallevents[ep][y] = Qpeakalleventstemp;
                        Qpeakalleventstemp = Qpeakalleventstemp2;
                    }
                }

                x = eventsnr[ep];
            }

    return (err);
} /* end hydromaxevents.c */


