/*
 *  HydroPrintTable.c
 *
 *  Prints the various summary values to HYDRO.LOG.
 *  Prints daily discharge to HYDRO.Q
 *  Prints daily sediment load data to HYDRO.QS
 *  for a user specified range of years.
 *
 *  Author1:    M.D. Morehead  (June 1998)
 *  Author2:    S.D. Peckham   (Jan 2002)
 *  Author3:    A.J. Kettner   (October 2002)(February 2003)
 *
 */

#include "hydroclimate.h"
#include "hydroparams.h"
#include "hydrotimeser.h"
#include "hydroinout.h"

/*----------------------------
 *  Start of HydroPrintTable
 *----------------------------*/
int
hydroprinttable()
{
    /*-------------------
     *  Local Variables
     *-------------------*/
    int err, ii, p;
    double baseflowpercentage;

    err = 0;
    baseflowpercentage = ((Qbartotal[ep] - baseflowtot[ep]) / Qbartotal[ep]);

    /*-------------------------------------------------
     *  Print Summary Table for the year to HYDRO.LOG
     *-------------------------------------------------*/
    fprintf(fidlog, " HydroPrintTable output: \n");
    fprintf(fidlog, " -----------------------\n");
    fprintf(fidlog, "\n");
    fprintf(fidlog, "   %s \n", title[ep]);
    fprintf(fidlog, "   Epoch = %d, Year = %d \n", ep + 1, yr);


    fprintf(fidlog, "   Hypsometric Integral by Digitized Function: \n");
    fprintf(fidlog, "\t using %d number of points \n\n", nhypts[ep]);

    fprintf(fidlog, "   Total Suspended Load: \t %.1f (10^6 kg/year) \n", Qsannual / 1e6);
    fprintf(fidlog, "   Total Bed Load: \t %.1f (10^6 kg/year) \n\n", Qbedannual / 1e6);

    fprintf(fidlog, "   Climate Trend and Basin Parameters: \n");
    fprintf(fidlog, "   ----------------------------------- \n");
    fprintf(fidlog, "\t\t\t Annual Precipitation \t\t Annual Temperature \n");
    fprintf(fidlog, "\t Starting Value: \t %.3f (m) \t\t %.3f (degC) \n", Pstart[ep],
        Tstart[ep]);
    fprintf(fidlog, "\t Change per Year: \t %.3f (m/a) \t\t %.3f (degC/a) \n", Pchange[ep],
        Tchange[ep]);
    fprintf(fidlog, "\t Std. Deviation: \t %.3f (m) \t\t %.3f (degC) \n", Pstd[ep],
        Tstd[ep]);
    fprintf(fidlog, "\t This Year: \t\t %.3f (m) \t\t %.3f (degC) \n", Pannual, Tannual);
    fprintf(fidlog, "\t baseflow: \t\t %.3f (m) \n\n", baseflowtot[ep]*dTOs * daysiy);

    fprintf(fidlog, "\t Max. Basin Altitude: \t %.1f (m) \n", maxalt[ep]);
    fprintf(fidlog, "\t ELA Starting Value: \t %.1f (m) \n", ELAstart[ep]);
    fprintf(fidlog, "\t ELA Change per Year: \t %.4f (m/a) \n", ELAchange[ep]);
    fprintf(fidlog, "\t ELA this Year:   \t %.1f (m) \n\n", ela);

    fprintf(fidlog, "\t Total Basin Area: \t\t %.1f (km^2) \n", totalarea[ep] / 1e6);
    fprintf(fidlog, "\t Glaciated Area above the ELA: \t %.1f (km^2) \n", smallg / 1e6);
    fprintf(fidlog, "\t Glaciated Area below the ELA: \t %.1f (km^2) \n\n", bigg / 1e6);

    fprintf(fidlog, "   Mass Balance Summary: \n");
    fprintf(fidlog, "   --------------------- \n");
    fprintf(fidlog,
        "   Minput =  Pannual + gwlast + previousice + previousnival + MQprevious \n");
    fprintf(fidlog, " \t Minput \t = \t\t\t %e (m^3) \n",            Minput);
    fprintf(fidlog, " \t Pannual \t = %e (m) \t %e (m^3)\n",     Pannual,
        Pannual * totalarea[ep]);
    fprintf(fidlog, " \t gwlast \t = \t\t\t %e (m^3) \n",            gwlast);
    fprintf(fidlog, " \t Gmass \t\t = \t\t\t %e (m^3) \n",       Gmass);
    fprintf(fidlog, " \t Msnowstart \t = \t\t\t %e (m^3) \n",        Msnowstart);
    fprintf(fidlog, " \t MQprevious \t = \t\t\t %e (m^3) \n\n",      MQprevious);


    fprintf(fidlog,
        "   Moutput =  MEtotal + Qtotal + gwstore + nextice + nextnival + MQnext \n");
    fprintf(fidlog, " \t Moutput \t = \t %e (m^3) \n",       Moutput);
    fprintf(fidlog, " \t MEtotal \t = \t %e (m^3)\n",        MEtotal);
    fprintf(fidlog, " \t Qtotal  \t = \t %e (m^3) \n",       Qgrandtotal[ep]);
    fprintf(fidlog, " \t gwstore[daysiy-1] = \t %e (m^3) \n",    gwstore[daysiy - 1]);
    fprintf(fidlog, " \t Msnowend \t = \t %e (m^3) \n",      Msnowend);
    fprintf(fidlog, " \t MQnext \t = \t %e (m^3) \n\n",      MQnext);

    /*-----------------------------------
     *  Print Discharge data to HYDRO.Q
     *-----------------------------------*/
    if (yr == tblstart[ep]) {
        fprintf(fidq,
            "%%\n%%================================================================\n");
        fprintf(fidq, "%%Epoch = %d, Year = %d \n%%\n", ep + 1, yr);

        fprintf(fidq, "%%Daily Discharge Results: \n");
        fprintf(fidq, "%%------------------------ \n");

        if (exceedflood > 0) {
            fprintf(fidq, "%%********* NOTE: Some peak flows are exceeding the \t\t ********* \n");
            fprintf(fidq,
                "%%********* Maximum Basin Flood Flow Rate of %.1f (m^3/s) \t ********* \n%%\n",
                maxflood);
        }

        fprintf(fidq, "%%All discharges in m^3/s \n%%\n");

        fprintf(fidq, "%%Daily baseflow = %.1f \n%%\n", baseflowtot[ep]);

        fprintf(fidq,
            "%%Day \t Qrain \t\t Qnival\t\t Qice \t\t Qss \t\t Qexceedgw \t\t gwstore \t Qtotal   \t");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidq, "Qoutlet:%d\t", p + 1);
            }

        fprintf(fidq, "\n");
        fprintf(fidq,
            "%%--- \t ----- \t\t ------\t\t ---- \t\t --- \t\t --------- \t\t -------- \t---------\t ");

        for (p = 0; p < maxnoutlet; p++) {
            fprintf(fidq, "---------\t");
        }

        fprintf(fidq, "\n%%\n");
    }

    for (ii = 0; ii < daysiy; ii++) {
        fprintf(fidq,
            "%d \t %.1f    \t %.1f    \t %.1f    \t %.1f    \t %.1f   \t\t\t %.1f\t %.2f\t\t ", \
            (yr - syear[ep]) * 365 + ii + 1, baseflowpercentage * Qrain[ii],
            baseflowpercentage * Qnival[ii], baseflowpercentage * Qice[ii],
            baseflowpercentage * Qss[ii], baseflowpercentage * Qexceedgw[ii], gwstore[ii],
            Qsumtot[ii]);

        for (p = 0; p < maxnoutlet; p++) {
            fprintf(fidq, "%.2f \t\t", Qsum[ii][p]);
        }

        fprintf(fidq, "\n");
    }

    /*----------------------------------------
     *  Print Sediment load data to HYDRO.QS
     *----------------------------------------*/
    if (yr == tblstart[ep]) {
        fprintf(fidqs,
            "%%\n%%================================================================\n");
        fprintf(fidqs, "%%Epoch = %d, Year = %d \n%%\n", ep + 1, yr);

        fprintf(fidqs, "%%Daily Sediment Load Results: \n");
        fprintf(fidqs, "%%------------------------ \n");

        fprintf(fidqs, "%%\n%%Day\tQtotal\t");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "Qoutl:%d\t", p + 1);
            }

        fprintf(fidqs, "Cstotal\t\t");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "Csoutl:%d\t", p + 1);
            }

        fprintf(fidqs, "Log(Qstot\t");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "Log(Qsoutl:%d)\t", p + 1);
            }

        fprintf(fidqs, "Bedloadtot\t");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "Bedloadoutl:%d\t", p + 1);
            }

        fprintf(fidqs, "\n");

        fprintf(fidqs, "%%   \t(m^3/s) ");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "(m^3/s) ");
            }

        fprintf(fidqs, "(kg/m^3) \t");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "(kg/m^3) \t");
            }

        fprintf(fidqs, "(kg/s) \t\t\t");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "(kg/s) \t\t\t");
            }

        fprintf(fidqs, "(kg/s) \t\t\t");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "(kg/s) \t\t\t");
            }

        fprintf(fidqs, "\n");

        fprintf(fidqs, "%%--- \t------- ");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "------- ");
            }

        fprintf(fidqs, "-------- \t");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "-------- \t");
            }

        fprintf(fidqs, "------ \t\t\t");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "------ \t\t\t");
            }

        fprintf(fidqs, "-------- \t\t");

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "-------- \t\t");
            }

        fprintf(fidqs, "\n%%\n");
    }

    for (ii = 0; ii < daysiy; ii++) {
        fprintf(fidqs, "%d  \t%.2f    ", (yr - syear[ep]) * 365 + ii + 1, Qsumtot[ii]);

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "%.2f    ", Qsum[ii][p]);
            }

        fprintf(fidqs, "%.4f \t\t", Cs[ii]);

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "%.4f \t", Csoutlet[ii][p]);
            }

        fprintf(fidqs, "%.1f    \t", log(Qs[ii]));

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "%.1f    \t", log(Qsoutlet[ii][p]));
            }

        fprintf(fidqs, "%.1f  \t", Qb[ii]);

        if (outletmodelflag == 1)
            for (p = 0; p < maxnoutlet; p++) {
                fprintf(fidqs, "%.1f  \t", Qboutlet[ii][p]);
            }

        fprintf(fidqs, "\n");
    }

    return (err);
}  /* end of HydroPrintTable.c */

