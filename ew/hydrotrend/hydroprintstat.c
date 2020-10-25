/*
 * HydroPrintStat.c Prints the statistics of the
 *          stochastic models used to calculate sediment
 *          discharge and discharge.
 *
 *          Author:     A.J. Kettner
 *          Original:   February 2003
 *
 * Variable     Def.Location        Type    Units   Usage
 * --------     ------------        ----    -----   -----
 * A            HydroPrintStat.c    double  km^2    total basin area
 * cbar         HydroPrintStat.c    double  -       mean of normal random variable
 * err          various             int     -       error flag, halts program
 * H            HydroPrintStat.c    double  m       max relief basin area
 * sigmapsi     HydroPrintStat.c    double  -       sigma of psi
 * Tbar         HydroPrintStat.c    double  Celsius mean basin temperature
 * s            HydroPrintStat.c    double  -       sigma C
 *
 */

#include <math.h>
#include "hydroclimate.h"
#include "hydroparams.h"
#include "hydroinout.h"

/*-----------------------------
 *  Start of HydroPrintStat
 *-----------------------------*/
int
hydroprintstat()
{

    /*-------------------
     *  Local Variables
     *-------------------*/
    int err;
    double A, H, Tbar, sigmapsi, cbar, s;

    /*------------------------
     *  Initialize variables
     *------------------------*/
    err = 0;

    /*----------------------------------
     *  Calculate the needed variables
     *----------------------------------*/
    A    = (totalarea[ep] / 1e6);
    H    = maxalt[ep];
    Tbar = Tstart[ep] - ((lapserate[ep] * maxalt[ep]) / 3.0);
    sigmapsi  = 0.763 * pow(0.99995, Qbartotal[ep]);
    cbar = (1.4 - (0.025 * Tbar) + (0.00013 * H) + (0.145 * log(Qsbartot[ep])));
    s    = 0.17 + (0.0000183 * Qbartotal[ep]);

    /*--------------------
     *  Print statistics
     *--------------------*/
    fprintf(fidstat, " Values used to calculate discharge and sediment discharge\n");
    fprintf(fidstat, " for the stochastic model in hydrotrend run: \n");
    fprintf(fidstat, " %s\n", title[ep]);
    fprintf(fidstat, "epoch: %d \n", ep + 1);
    fprintf(fidstat, "*****************************************************************\n");

    if (Qsbarformulaflag[ep] == 1) {
        fprintf(fidstat, "Qsbar =  alpha3 * pow(A,alpha4) * pow(H,alpha5) * exp(k * Tbar)\n");
        fprintf(fidstat, "\t A = %.2f; river basin area (km2)\n", A);
        fprintf(fidstat, "\t H = %.2f; maxalt = basin relief (m)\n\n", H);
        fprintf(fidstat, "\t T = %.2f; Tbar = mean basin temp (C)\n", Tbar);
        fprintf(fidstat,
            "\t\t T =Tstart[ep](=%.2f) - ((lapserate[ep](=%.2f) * maxalt[ep](=%.2f))/3.0\n\n",
            Tstart[ep], lapserate[ep], maxalt[ep]);
        fprintf(fidstat, "\t alpha3, alpha4, alpha5 and k are set by temperature and\n");
        fprintf(fidstat, "\t latitude geographic position of the river mouth, lat=%.2f\n ",
            lat);
        fprintf(fidstat, "\t alpha3 = %.2e\n", alpha3);
        fprintf(fidstat, "\t alpha4 = %.2e\n", alpha4);
        fprintf(fidstat, "\t alpha5 = %.2e\n", alpha5);
        fprintf(fidstat, "\t k= %.2e\n", k1);
        fprintf(fidstat, "Qsbar = %.2f (kg/s)\n\n\n", Qsbartot[ep]);
        fprintf(fidstat, "Qbar = (sumQ(daily))/number of days\n");
        fprintf(fidstat, "Qbar = %.2f (m3/s)\n\n\n", Qbartotal[ep]);
    }

    if (Qsbarformulaflag[ep] == 0) {
        fprintf(fidstat,
            "Qsbar =  alpha3 * pow(Qbar,alpha4) * pow(H,alpha5) * exp(k * Tbar)\n");
        fprintf(fidstat, "\t Qbar = %.2f; long-term average of Q (m3/s)\n", Qbartotal[ep]);
        fprintf(fidstat, "\t H = %.2f; maxalt = basin relief (m)\n\n", H);
        fprintf(fidstat, "\t T = %.2f; Tbar = mean basin temp (C)\n", Tbar);
        fprintf(fidstat,
            "\t\t T =Tstart[ep](=%.2f) - ((lapserate[ep](=%.2f) * maxalt[ep](=%.2f))/3.0\n\n",
            Tstart[ep], lapserate[ep], maxalt[ep]);
        fprintf(fidstat, "\t alpha6, alpha7, alpha8 and k are set by temperature and\n");
        fprintf(fidstat, "\t latitude geographic position of the river mouth, lat=%.2f\n ",
            lat);
        fprintf(fidstat, "\t alpha6 = %.2e\n", alpha6);
        fprintf(fidstat, "\t alpha7 = %.2e\n", alpha7);
        fprintf(fidstat, "\t alpha8 = %.2e\n", alpha8);
        fprintf(fidstat, "\t k= %.2e\n", k2);
        fprintf(fidstat, "Qsbar = %.2f (kg/s)\n\n\n", Qsbartot[ep]);
    }

    fprintf(fidstat, "(Qs(daily)/Qsbar) = psi(daily) * (Q(daily)/Qbar)^C(daily)\n");
    fprintf(fidstat, "\t Qs(daily) = daily sediment discharge (kg/s)\n");
    fprintf(fidstat,
        "\t Qsbar = %.2f (kg/s); used long-term average of Qs (fractions of Qs(daily)/nr. of fractions)\n\n",
        Qsmean[ep]*Qsbarnew[ep]);
    fprintf(fidstat, "\t psi = log-normal random variable,\n");
    fprintf(fidstat, "\t\t psi   = a random number from a lognormal distribution with\n");
    fprintf(fidstat, "\t\t mean 1 and sigma psi = 0.763 * (0.99995^Qbar)\n");
    fprintf(fidstat, "\t sigma psi = %f\n\n", sigmapsi);
    fprintf(fidstat, "\t Q(daily) = daily discharge (m3/s)\n");
    fprintf(fidstat,
        "\t Qbar = %.2f (m3/s); long-term average of Q, calculated by formula explained above\n\n",
        Qbartotal[ep]);
    fprintf(fidstat, "\t C\t= a random number from a distribution with mean E(C)\n");
    fprintf(fidstat, "\t\t and standard deviation sigma-C, where:\n");
    fprintf(fidstat, "\t\t E(C) = (1.4 - (0.025*T) + (0.00013*H) + (0.145*ln(Qsbar))\n");
    fprintf(fidstat, "\t\t E(C) = %f\n", cbar);
    fprintf(fidstat, "\t\t sigma-C = 0.17 + (0.0000183 * Qbar)\n");
    fprintf(fidstat, "\t\t sigma-C = %f\n\n", s);
    fprintf(fidstat,
        "*****************************************************************\n\n\n");
    return (err);
}   /* end of hydroprintstat.c */
