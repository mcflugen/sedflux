/*
 *  Hydrocalqsnew.c
 *
 *  Calculates the Qsbarnew.
 *  Author:    A.J. Kettner  (January 2003)
 *
 *
 *  Variable    Def.Location    Type    Units   Usage
 *  --------    ------------    ----    -----   -----
 *  err         various         int     -       error flag, halts program
 *  p           various         int     -       counter for the outlets
 *  Qsgrandtotaloutlettot Hydrocalqsnew.c double kg/s total Qs of all outlets
 *
 */

#include <stdio.h>
#include "hydroparams.h"
#include "hydrotimeser.h"
#include "hydroclimate.h"
#include "hydroinout.h"

/*----------------------
 *  Start main program
 *----------------------*/
int
hydrocalqsnew(void)
{

    /*-------------------
     *  Local Variables
     *-------------------*/
    int err, p;
    double Qsgrandtotaloutlettot;

    /*------------------------
     *  Initialize variables
     *------------------------*/
    err  = 0;
    Qsgrandtotaloutlettot = 0.0;

    /*--------------------------------------------------
     *  Calculate mean Qs and calculate the difference
     *  between mean Qs and Qsbar.
     *--------------------------------------------------*/
    Qsmean[ep]  = Qsgrandtotal[ep] / (nyears[ep] * daysiy * dTOs);
    Qsbarnew[ep] = Qsbartot[ep] / Qsmean[ep];

    if (outletmodelflag == 1) {
        for (p = 0; p < maxnoutlet; p++) {
            Qsgrandtotaloutlettot += Qsgrandtotaloutlet[ep][p];
        }

        Qsbarnew2[ep] = (1.0 -  sedfilter[ep]) * Qsbartot[ep] / (Qsgrandtotaloutlettot /
                (nyears[ep] * daysiy * dTOs));

        if (Qsbarnew2[ep] == 0.0) {
            printf("Qsbartot=%f, Qsgrandtot=%f, time=%f\n", Qsbartot[ep], Qsgrandtotaloutlettot,
                nyears[ep]*daysiy * dTOs);
        }
    }

    return (err);

}  /* end of Hydrocalqsnew */

