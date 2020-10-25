/*
 *  HydroOutput.c
 *
 *  Writes the discharge (v,w,d) and sediment load
 *  data to a binary file.  After averaging the data
 *  to the user requested timestep (d,m,s,y).
 *
 *  Author1:   M.D. Morehead  (June 1998)
 *  Author2:   S.D. Peckham   (January 2002)
 *  Author3:   A.J. Kettner   (September 2002) (February 2003)
 *
 * Variable     Def.Location    Type    Units   Usage
 * --------     ------------    ----    -----   -----
 * comLen       HydroOutput.c   int     -       length of the title string
 * Cs           HydroOutput.c   float   kg/m^3  averaged suspended concentration per grain class
 * Cstot        HydroOutput.c   float   kg/m^3  averaged suspended concentration total
 * dep[]        HydroOutput.c   float   m       river depth
 * err          various         int     -       error flag, halts program
 * ii           various         int     -       temporary loop counter
 * jj           various         int     -       temporary loop counter
 * kk           various         int     -       temporary loop counter
 * nrecords     HydroOutput.c   int     -       total # of output records
 * nYears       HydroOutput.c   int     -       total # of output years
 * recperyear   HydroOutput.c   int     -       # of output records per year
 * Qavg[]       HydroOutput.c   float   m^3/s   average river discharge
 * Qbavg[]      HydroOutput.c   float   kg/s    averaged bedload discharge
 * Qsavg[]      HydroOutput.c   float   kg/s    averaged suspended load flux
 * vel[]        HydroOutput.c   float   m/s     river velocity
 * wid[]        HydroOutput.c   float   m       river width
 *
 */

#include <string.h>
#include "hydroclimate.h"
#include "hydroparams.h"
#include "hydrotimeser.h"
#include "hydroinout.h"
#include "hydroreadclimate.h"
#include "hydroalloc_mem.h"
#include "hydrofree_mem.h"
#include "hydrodaysmonths.h"

/*------------------------
 *  Start of HydroOutput
 *------------------------*/
int
hydrooutput()
{
    static int daystrs[4] = { 1,  91, 182, 274};
    static int dayends[4] = {90, 181, 273, 365};
    static int daysis[4]  = {90,  91,  92,  92};

    /*-------------------
     *  Local Variables
     *-------------------*/
    int ii, jj, kk, p;
    int err, comLen, nrecords, nYears;

    static int recperyear;

    float   vel[daysiy], wid[daysiy], dep[daysiy];
    float**   veloutlet, **widoutlet, **depoutlet;
    float   Qavg[daysiy], Qbavg[daysiy], **Qavgoutlet, **Qbavgoutlet;
    float   Qsavg[daysiy], **Qsavgoutlet;
    float   Cs, Cstot, *Csoutlet, *Cstotoutlet;
    Cstot = 0.0;
    err = 0;

    /*---------------------------------------
     *  Allocate memory for multiple outlet
     *---------------------------------------*/
    veloutlet           = malloc2d(daysiy, maxnoutlet, float);
    widoutlet           = malloc2d(daysiy, maxnoutlet, float);
    depoutlet           = malloc2d(daysiy, maxnoutlet, float);
    Qavgoutlet          = malloc2d(daysiy, maxnoutlet, float);
    Qbavgoutlet         = malloc2d(daysiy, maxnoutlet, float);
    Qsavgoutlet         = malloc2d(daysiy, maxnoutlet, float);
    Csoutlet            = malloc1d(maxnoutlet, float);
    Cstotoutlet         = malloc1d(maxnoutlet, float);

    /*------------------------------------------------
     *  Print the header to the binary file (fiddis)
     *  Opened in HydroOpenFiles.c
     *  Closed in HydroTrend.c
     *------------------------------------------------*/
    if (yr == syear[0]) {
        if (timestep[0] == 'd') {
            recperyear = 365;
        } else if (timestep[0] == 'm') {
            recperyear = 12;
        } else if (timestep[0] == 's') {
            recperyear = 4;
        } else {
            recperyear = 1;
        }

        nYears   = syear[nepochs - 1] + nyears[nepochs - 1] - syear[0];
        comLen   = strlen(title[0]) - 1;
        nrecords = nYears * recperyear;
        fwrite(&comLen,  sizeof(int),      1, fiddistot);
        fwrite(title[0], sizeof(char), comLen, fiddistot);
        fwrite(&ngrain,  sizeof(int),      1, fiddistot);
        fwrite(&recperyear,  sizeof(int),      1, fiddistot);
        fwrite(&nrecords,  sizeof(int),      1, fiddistot);

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fwrite(&comLen,  sizeof(int),      1, fiddis[p]);
                fwrite(title[0], sizeof(char), comLen, fiddis[p]);
                fwrite(&ngrain,  sizeof(int),      1, fiddis[p]);
                fwrite(&recperyear,  sizeof(int),      1, fiddis[p]);
                fwrite(&nrecords,  sizeof(int),      1, fiddis[p]);
            }
    }

    /*-------------------------------------
     *  Average over the desired interval
     *-------------------------------------*/
    if (timestep[0] == 'd') {

        /*------------------------------------------
         *  Calculate V,W,D.
         *  Daily Qb and Cs are already calculated
         *------------------------------------------*/
        for (jj = 0; jj < recperyear; jj++) {
            Qavg[jj]    = (float)(Qsumtot[jj]);
            vel[jj]     = (float)(velcof[ep] * pow(Qavg[jj], velpow[ep]));
            wid[jj]     = (float)(widcof[ep] * pow(Qavg[jj], widpow[ep]));
            dep[jj]     = (float)(depcof[ep] * pow(Qavg[jj], deppow[ep]));
            Qbavg[jj]   = (float)(Qb[jj]);
            Qsavg[jj]   = (float)(Qs[jj]);

            if (outletmodelflag == 1)
                for (p = 0; p < maxnoutlet; p++) {
                    Qavgoutlet[jj][p]       = (float)(Qsum[jj][p]);
                    veloutlet[jj][p]        = (float)(velcof[ep] * pow(Qavgoutlet[jj][p], velpow[ep]));
                    widoutlet[jj][p]        = (float)(widcof[ep] * pow(Qavgoutlet[jj][p], widpow[ep]));
                    depoutlet[jj][p]        = (float)(depcof[ep] * pow(Qavgoutlet[jj][p], deppow[ep]));
                    Qbavgoutlet[jj][p]  = (float)(Qboutlet[jj][p]);
                    Qsavgoutlet[jj][p]  = (float)(Qsoutlet[jj][p]);
                }
        }
    } else if (timestep[0] == 'm') {
        /*-----------------------------------------------------------
         *  Average over each month and then compute monthly values
         *-----------------------------------------------------------*/
        //  for (jj=0; jj<recperyear; jj++)
        for (jj = Jan; jj <= Dec; jj++) {
            Qavg[jj] = 0.0;
            Qbavg[jj] = 0.0;
            Qsavg[jj] = 0.0;

            if (outletmodelflag == 1)
                for (p = 0; p < maxnoutlet; p++) {
                    Qavgoutlet[jj][p]   = 0.0;
                    Qbavgoutlet[jj][p]  = 0.0;
                    Qsavgoutlet[jj][p]  = 0.0;
                }

            //      for( ii=daystrm[jj]; ii<dayendm[jj]; ii++ )
            for (ii = start_of(jj); ii < end_of(jj); ii++) {
                Qavg[jj]    += (float)(Qsumtot[ii]);
                Qbavg[jj]   += (float)(Qb[ii]);
                Qsavg[jj]   += (float)(Qs[ii]);

                if (outletmodelflag == 1)
                    for (p = 0; p < maxnoutlet; p++) {
                        Qavgoutlet[jj][p]   += (float)(Qsum[ii][p]);
                        Qbavgoutlet[jj][p]  += (float)(Qboutlet[ii][p]);
                        Qsavgoutlet[jj][p]  += (float)(Qsoutlet[ii][p]);
                    }
            }

            //      Qavg[jj]    = Qavg[jj]/daysim[jj];
            Qavg[jj]    = Qavg[jj] / days_in_month(jj);
            vel[jj] = (float)(velcof[ep] * pow(Qavg[jj], velpow[ep]));
            wid[jj] = (float)(widcof[ep] * pow(Qavg[jj], widpow[ep]));
            dep[jj] = (float)(depcof[ep] * pow(Qavg[jj], deppow[ep]));
            //      Qbavg[jj]= Qbavg[jj]/daysim[jj];
            Qbavg[jj] = Qbavg[jj] / days_in_month(jj);
            //      Qsavg[jj]= Qsavg[jj]/daysim[jj];
            Qsavg[jj] = Qsavg[jj] / days_in_month(jj);

            if (outletmodelflag == 1)
                for (p = 0; p < maxnoutlet; p++) {
                    //              Qavgoutlet[jj][p]   = Qavgoutlet[jj][p]/daysim[jj];
                    Qavgoutlet[jj][p]   = Qavgoutlet[jj][p] / days_in_month(jj);
                    veloutlet[jj][p]    = (float)(velcof[ep] * pow(Qavgoutlet[jj][p], velpow[ep]));
                    widoutlet[jj][p]    = (float)(widcof[ep] * pow(Qavgoutlet[jj][p], widpow[ep]));
                    depoutlet[jj][p]    = (float)(depcof[ep] * pow(Qavgoutlet[jj][p], deppow[ep]));
                    //              Qbavgoutlet[jj][p]  = Qbavgoutlet[jj][p]/daysim[jj];
                    Qbavgoutlet[jj][p]  = Qbavgoutlet[jj][p] / days_in_month(jj);
                    //              Qsavgoutlet[jj][p]  = Qsavgoutlet[jj][p]/daysim[jj];
                    Qsavgoutlet[jj][p]  = Qsavgoutlet[jj][p] / days_in_month(jj);
                }
        }
    } else if (timestep[0] == 's') {

        /*-------------------------------------------------------------
         *  Average over each season and then compute seasonal values
         *-------------------------------------------------------------*/
        for (jj = 0; jj < recperyear; jj++) {
            Qavg[jj]    = 0.0;
            Qbavg[jj]   = 0.0;
            Qsavg[jj]   = 0.0;

            if (outletmodelflag == 1)
                for (p = 0; p < maxnoutlet; p++) {
                    Qavgoutlet[jj][p]   = 0.0;
                    Qbavgoutlet[jj][p]  = 0.0;
                    Qsavgoutlet[jj][p]  = 0.0;
                }

            for (ii = daystrs[jj]; ii < dayends[jj]; ii++) {
                Qavg[jj]    += (float)(Qsumtot[ii]);
                Qbavg[jj]   += (float)(Qb[ii]);
                Qsavg[jj]   += (float)(Qs[ii]);

                if (outletmodelflag == 1)
                    for (p = 0; p < maxnoutlet; p++) {
                        Qavgoutlet[jj][p]   += (float)(Qsum[ii][p]);
                        Qbavgoutlet[jj][p]  += (float)(Qboutlet[ii][p]);
                        Qsavgoutlet[jj][p]  += (float)(Qsoutlet[ii][p]);
                    }
            }

            Qavg[jj]    = Qavg[jj] / daysis[jj];
            vel[jj]     = (float)(velcof[ep] * pow(Qavg[jj], velpow[ep]));
            wid[jj]     = (float)(widcof[ep] * pow(Qavg[jj], widpow[ep]));
            dep[jj]     = (float)(depcof[ep] * pow(Qavg[jj], deppow[ep]));
            Qbavg[jj]   = Qbavg[jj] / daysis[jj];
            Qsavg[jj]   = Qsavg[jj] / daysis[jj];

            if (outletmodelflag == 1)
                for (p = 0; p < maxnoutlet; p++) {
                    Qavgoutlet[jj][p]   = Qavgoutlet[jj][p] / daysis[jj];
                    veloutlet[jj][p]    = (float)(velcof[ep] * pow(Qavgoutlet[jj][p], velpow[ep]));
                    widoutlet[jj][p]    = (float)(widcof[ep] * pow(Qavgoutlet[jj][p], widpow[ep]));
                    depoutlet[jj][p]    = (float)(depcof[ep] * pow(Qavgoutlet[jj][p], deppow[ep]));
                    Qbavgoutlet[jj][p] = Qbavgoutlet[jj][p] / daysis[jj];
                    Qsavgoutlet[jj][p] = Qsavgoutlet[jj][p] / daysis[jj];
                }
        }
    } else {

        /*---------------------------------------------------------
         *  Average over each year and then compute annual values
         *---------------------------------------------------------*/
        jj = 0;
        Qavg[jj]    = 0.0;
        Qbavg[jj]   = 0.0;
        Qsavg[jj]   = 0.0;

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                jj = 0;
                Qavgoutlet[jj][p]   = 0.0;
                Qbavgoutlet[jj][p]  = 0.0;
                Qsavgoutlet[jj][p]  = 0.0;
            }

        for (ii = 0; ii < daysiy; ii++) {
            Qavg[jj]    += (float)(Qsumtot[ii]);
            Qbavg[jj]   += (float)(Qb[ii]);
            Qsavg[jj]   += (float)(Qs[ii]);

            if (outletmodelflag == 1)
                for (p = 0; p < maxnoutlet; p++) {
                    Qavgoutlet[jj][p]       += (float)(Qsum[ii][p]);
                    Qbavgoutlet[jj][p]      += (float)(Qboutlet[ii][p]);
                    Qsavgoutlet[jj][p]      += (float)(Qsoutlet[ii][p]);
                }
        }

        Qavg[jj]    = Qavg[jj] / daysiy;
        vel[jj]     = (float)(velcof[ep] * pow(Qavg[jj], velpow[ep]));
        wid[jj]     = (float)(widcof[ep] * pow(Qavg[jj], widpow[ep]));
        dep[jj]     = (float)(depcof[ep] * pow(Qavg[jj], deppow[ep]));
        Qbavg[jj]   = Qbavg[jj] / daysiy;
        Qsavg[jj]   = Qsavg[jj] / daysiy;

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                Qavgoutlet[jj][p]   = Qavgoutlet[jj][p] / daysiy;
                veloutlet[jj][p]    = (float)(velcof[ep] * pow(Qavgoutlet[jj][p], velpow[ep]));
                widoutlet[jj][p]    = (float)(widcof[ep] * pow(Qavgoutlet[jj][p], widpow[ep]));
                depoutlet[jj][p]    = (float)(depcof[ep] * pow(Qavgoutlet[jj][p], deppow[ep]));
                Qbavgoutlet[jj][p]  = Qbavgoutlet[jj][p] / daysiy;
                Qsavgoutlet[jj][p]  = Qsavgoutlet[jj][p] / daysiy;
            }
    }  /* end if-else for time interval */

    /*
     *  Print the data for each year to the binary file
     *      velocity
     *      width
     *      depth
     *      bedload
     *      *conc[ii]
     */
    for (jj = 0; jj < recperyear; jj++) {
        fwrite(&vel[jj], sizeof(float), 1, fiddistot);
        fwrite(&wid[jj], sizeof(float), 1, fiddistot);
        fwrite(&dep[jj], sizeof(float), 1, fiddistot);
        fwrite(&Qbavg[jj], sizeof(float), 1, fiddistot);

        for (kk = 0; kk < ngrain; kk++) {
            Cs = (float)(grainpct[kk][ep] * Qsavg[jj] / Qavg[jj]);

            if (Qavg[jj] == 0.0) {
                Cs = 0.0;
            }

            fwrite(&Cs, sizeof(float), 1, fiddistot);
        }

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fwrite(&veloutlet[jj][p], sizeof(float), 1, fiddis[p]);
                fwrite(&widoutlet[jj][p], sizeof(float), 1, fiddis[p]);
                fwrite(&depoutlet[jj][p], sizeof(float), 1, fiddis[p]);
                fwrite(&Qbavgoutlet[jj][p], sizeof(float), 1, fiddis[p]);

                for (kk = 0; kk < ngrain; kk++) {
                    Csoutlet[p] = (float)(grainpct[kk][ep] * Qsavgoutlet[jj][p] / Qavgoutlet[jj][p]);

                    if (Qavgoutlet[jj][p] == 0.0) {
                        Csoutlet[p] = 0.0;
                    }

                    fwrite(&Csoutlet[p], sizeof(float), 1, fiddis[p]);
                }
            }
    }

    /*----------------------------------------------
     *ascii routine to write the dis file in ascii
     *----------------------------------------------*/
    if (strncmp(asciioutput, ON, 2) == 0) {
        if (yr == syear[0]) {
            fprintf(outp, "vel(m/s) wid(m) dep(m) ");
            fprintf(outp1, "Q(m^3/s) ");
            fprintf(outp2, "Cs(kg/m^3) Qs(kg/s) ");
            fprintf(outp3, "Qbavg() ");
            fprintf(outp4, "Cs_per_grainsize(kg/m^3) ");
            fprintf(outp5, "Qsavg() ");

            if (outletmodelflag == 1)
                for (p = 0; p < maxnoutlet; p++) {
                    fprintf(outp, "vel(m/s) wid(m) dep(m) ");
                    fprintf(outp1, "Q(m^3/s) ");
                    fprintf(outp2, "Cs(kg/m^3) Qs(kg/s) ");
                    fprintf(outp3, "Qbavg() ");
                    fprintf(outp4, "Cs_per_grainsize(kg/m^3) ");
                    fprintf(outp5, "Qsavg(kg/s) ");
                }

            fprintf(outp, "\n");
            fprintf(outp1, "\n");
            fprintf(outp2, "\n");
            fprintf(outp3, "\n");
            fprintf(outp4, "\n");
            fprintf(outp5, "\n");
            fprintf(outp, "--------  ------- ------- ");
            fprintf(outp1, "--------- ");
            fprintf(outp2, "----------  --------- ");
            fprintf(outp3, "-------- ");
            fprintf(outp4, "------------------------ ");
            fprintf(outp5, "-------- ");

            if (outletmodelflag == 1)
                for (p = 0; p < maxnoutlet; p++) {
                    fprintf(outp, "--------  ------- ------- ");
                    fprintf(outp1, "--------- ");
                    fprintf(outp2, "----------  --------- ");
                    fprintf(outp3, "-------- ");
                    fprintf(outp4, "------------------------ ");
                    fprintf(outp5, "-------- ");
                }

            fprintf(outp, "\n");
            fprintf(outp1, "\n");
            fprintf(outp2, "\n");
            fprintf(outp3, "\n");
            fprintf(outp4, "\n");
            fprintf(outp5, "\n");
        }

        for (jj = 0; jj < recperyear; jj++) {
            fprintf(outp, "%.3f\t  %.3f\t  %.3f\t  ", vel[jj], wid[jj], dep[jj]);
            fprintf(outp1, "%.3f ", vel[jj]*wid[jj]*dep[jj]);
            fprintf(outp3, "%.3f ", Qbavg[jj]);

            for (kk = 0; kk < ngrain; kk++) {
                if (kk == 0) {
                    Cstot = 0.0;
                }

                Cs = (float)(grainpct[kk][ep] * Qsavg[jj] / Qavg[jj]);

                if (Qavg[jj] == 0) {
                    Cs = 0.0;
                }

                Cstot += Cs;
                fprintf(outp4, "%.3f ", Cs);

                if (kk == ngrain - 1) {
                    fprintf(outp2, "%.3f\t %.3f ", Cstot, Cstot * vel[jj]*wid[jj]*dep[jj]);
                }
            }

            fprintf(outp5, "%.3f ", Qsavg[jj]);

            if (outletmodelflag == 1)
                for (p = 0; p < maxnoutlet; p++) {
                    fprintf(outp, "%.3f\t  %.3f\t  %.3f\t  ", veloutlet[jj][p], widoutlet[jj][p],
                        depoutlet[jj][p]);
                    fprintf(outp1, "%.3f ", veloutlet[jj][p]*widoutlet[jj][p]*depoutlet[jj][p]);
                    fprintf(outp3, "%.3f ", Qbavgoutlet[jj][p]);

                    for (kk = 0; kk < ngrain; kk++) {
                        if (kk == 0) {
                            Cstotoutlet[p] = 0.0;
                        }

                        Csoutlet[p] = (float)(grainpct[kk][ep] * Qsavgoutlet[jj][p] / Qavgoutlet[jj][p]);

                        if (Qavgoutlet[jj][p] == 0) {
                            Csoutlet[p] = 0.0;
                        }

                        Cstotoutlet[p] += Csoutlet[p];
                        fprintf(outp4, "%.3f ", Csoutlet[p]);

                        if (kk == ngrain - 1) {
                            fprintf(outp2, "%.3f %.3f ", Cstotoutlet[p],
                                Cstotoutlet[p]*veloutlet[jj][p]*widoutlet[jj][p]*depoutlet[jj][p]);
                        }
                    }

                    fprintf(outp5, "%.3f ", Qsavgoutlet[jj][p]);
                }

            fprintf(outp, "\n");
            fprintf(outp1, "\n");
            fprintf(outp2, "\n");
            fprintf(outp3, "\n");
            fprintf(outp4, "\n");
            fprintf(outp5, "\n");
        }
    }

    /*---------------
     *  Free memory
     *---------------*/
    freematrix2D((void**)veloutlet, daysiy);
    freematrix2D((void**)widoutlet, daysiy);
    freematrix2D((void**)depoutlet, daysiy);
    freematrix2D((void**)Qavgoutlet, daysiy);
    freematrix2D((void**)Qbavgoutlet, daysiy);
    freematrix2D((void**)Qsavgoutlet, daysiy);
    freematrix1D((void*)Csoutlet);
    freematrix1D((void*)Cstotoutlet);

    return (err);
}  /* end of HydroOutput.c */

