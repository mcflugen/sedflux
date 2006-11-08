/*
 * HydroPrintAnnual.c	Prints the annually averaged data to series of
 *			text files.  Used to view the long term trends.
 *
 *	Author1:		M.D. Morehead	(June 1998)
 *  Author2:		A.J. Kettner	(February 2003)
 *
 * Variable		Def.Location	Type	Units	Usage
 * --------		------------	----	-----	-----
 * err			various		int	-	error flag, halts program
 * ii			various		int	-	temporary loop counter
 * fef			HydroPrintTrend	int	-	maximum basin Flood Exceedance Flag
 * Qrt			HydroPrintTrend	double	m^3/a	total rain derived discharge per year
 * Qit			HydroPrintTrend	double	m^3/a	total glacial derived discharge per year
 * Qnt			HydroPrintTrend	double	m^3/a	total nival derived discharge per year
 * Qst			HydroPrintTrend	double	m^3/a	total subsurface storm flow derived discharge per year
 * Qbt			HydroPrintTrend	double	m^3/a	total baseflow derived discharge per year
 * Qet			HydroPrintTrend	double	m^3/a	total exceed groundwater derived discharge per year
 *
 */
#include "hydroclimate.h"
#include "hydroparams.h"
#include "hydrotimeser.h"
#include "hydroinout.h"

/*-----------------------------
 *  Start of HydroPrintAnnual
 *-----------------------------*/
int hydroprintannual()
{

/*-------------------
 *  Local Variables
 *-------------------*/
int err, ii, fef,p;
double	Qrt, Qit, Qnt, Qst, Qbt, Qet, baseflowpercentage;

baseflowpercentage = ((Qbartotal[ep] - baseflowtot[ep])/Qbartotal[ep]);

/*------------------------
 *  Initialize variables
 *------------------------*/
err = 0;
Qrt = 0.0;
Qit = 0.0;
Qnt = 0.0;
Qst = 0.0;
Qbt = 0.0;
Qet = 0.0;

/*----------------------------------
 *  Calculate the needed variables
 *----------------------------------*/
if( Qpeak > maxflood )
  fef = 1;
else
  fef = 0;

for( ii=0; ii<daysiy; ii++ ) {
   Qrt += Qrain[ii]*baseflowpercentage;
   Qit += Qice[ii]*baseflowpercentage;
   Qnt += Qnival[ii]*baseflowpercentage;
   Qst += Qss[ii]*baseflowpercentage;
   Qbt += baseflowtot[ep];
   Qet += Qexceedgw[ii]*baseflowpercentage;
}
Qrt *= dTOs;
Qit *= dTOs;
Qnt *= dTOs;
Qst *= dTOs;
Qbt *= dTOs;
Qet *= dTOs;

/*----------------------
 *  Print trend file 1
 *----------------------*/
if( yr == syear[ep] && ep == 0) {
   fprintf( fidtrend1, "%% Annual Summary for: \n" );
   fprintf( fidtrend1, "%% %s", title[ep] );
   fprintf( fidtrend1, "%% maxflood = %.1f (m^3/s) \n%%\n", maxflood );

/*                        1234 \t 123456 \t 123456 \t 123456 \t 123456 \t 12345 \t 123456 \t 123456 */
   fprintf( fidtrend1, "%%Year \t Avg.   \t Total  \t ELA    \t Glacier\t Flood  \t Baseflw\t Qpeak  \t");
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)   
	fprintf( fidtrend1, "Qpeak   \t");
   fprintf( fidtrend1, "\n" );
   fprintf( fidtrend1, "%%     \t Temp   \t Precip \t        \t Area   \t Exceed \t Precip \t for delta\t");
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)   
	fprintf( fidtrend1, "outlet:%d\t",p+1);
   fprintf( fidtrend1, "\n" );
   fprintf( fidtrend1, "%%     \t (degC) \t  (m)   \t (m)    \t (km^2) \t Flag   \t (m)    \t (m^3/s)\t");
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)   
	fprintf( fidtrend1, "(m^3/s)  \t");
   fprintf( fidtrend1, "\n" );
   fprintf( fidtrend1, "%%---- \t ------ \t ------ \t -------\t -------\t ------ \t ---    \t -------\t");
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)   
	fprintf( fidtrend1, "--------\t");
   fprintf( fidtrend1, "\n");
}

/*                  "Year \t Avg.   \t Total  \t ELA    \t Glacier\t Flood  \t Baseflw\t  Qpeak \t*/
/*                   1234 \t 123456 \t 123456 \t 123456 \t 123456 \t 12345  \t 123456 \t 123456 \t*/
fprintf( fidtrend1, "%d   \t %.2f   \t %.2f   \t %.1f   \t %.1f   \t\t %d     \t\t %.1f   \t\t %.1f   \t", \
      yr, Tannual, Pannual, ela, (bigg+smallg)/1e6, fef, baseflowtot[ep]*dTOs*daysiy, Qpeak);
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)   
			fprintf( fidtrend1, "%.1f\t\t",Qpeakperoutlet[p]);      
fprintf( fidtrend1, "\n");

/*----------------------
 *  Print trend file 2
 *----------------------*/
if( yr == syear[ep] && ep == 0 ) {
   fprintf( fidtrend2, "%% Annual Summary for: \n" );
   fprintf( fidtrend2, "%% %s", title[ep] );
   fprintf( fidtrend2, "%% discharges in 10^6 m^3/annum, ending GW pool in 10^6 m^3\n" );
   fprintf( fidtrend2, "%% to convert to mean annual, divide by 1e6*86400/365 -> m^3/s \n%%\n" );

/*                        1234 \t 123456 \t 123456 \t 123456 \t 123456 \t 123456 \t 123456 \t 123456 \t 123456 */
   fprintf( fidtrend2, "%%Year \t Qtotal \t Qrain  \t Qice   \t Qnival \t Qss    \t Qbase  \t Qexcdgw\t GWend \n");
   fprintf( fidtrend2, "%%---- \t ------ \t ------ \t ----   \t ------ \t ---    \t -----  \t -------\t ----- \n");
}

/*                   1234 \t 123456 \t 123456 \t 123456 \t 123456 \t 123456 \t 123456 \t 123456 \t 123456	*/
/*                "%%Year \t Qtotal \t Qrain  \t Qice   \t Qnival \t Qss    \t Qbase  \t Qexcdgw\t GWend	*/
fprintf( fidtrend2, "%d   \t %.1f   \t %.1f   \t %.1f   \t %.1f   \t %.1f   \t %.1f   \t %.1f   \t %.2e \n", \
                     yr, Qtotal/1e6, Qrt/1e6, Qit/1e6,  Qnt/1e6,  Qst/1e6,  Qbt/1e6,  Qet/1e6, gwstore[daysiy-1] );


/*----------------------
 *  Print trend file 3
 *----------------------*/
if( yr == syear[ep] && ep == 0) {
	fprintf( fidtrend3, "%% Annual Summary for: \n" );
	fprintf( fidtrend3, "%% %s", title[ep] );
	fprintf( fidtrend3, "%% discharge in 10^6 m^3/annum, Sediment load in 10^9 kg/annum \n%%\n" );
	fprintf( fidtrend3, "%%Year \t Qtotal \t ");
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)   
			fprintf( fidtrend3, "Qoutlet:%d \t ",p+1);		
	fprintf( fidtrend3, "Qstotal\t ");
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)   
			fprintf( fidtrend3, "Qsoutlet:%d\t ",p+1);
	fprintf( fidtrend3, "Qbedtotal\t ");
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)   
			fprintf( fidtrend3, "Qbedoutlet:%d \t ",p+1);
	fprintf( fidtrend3, "\n");	
	fprintf( fidtrend3, "%%Qfrac.\t (100%%) \t" );
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)
			fprintf( fidtrend3, " (%.0f%%)\t\t",outletpcttotevents[p][ep]*100);
	fprintf( fidtrend3, " (100%%)  \t ");		
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)
			fprintf( fidtrend3, "(%.0f%%)\t\t ",outletpcttotevents[p][ep]*100);
	fprintf( fidtrend3, "(100%%)    \t ");		
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)
			fprintf( fidtrend3, "(%.0f%%)\t\t     ",outletpcttotevents[p][ep]*100);
	fprintf( fidtrend3, "\n");
	fprintf( fidtrend3, "%%---- \t ------ \t" );
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)
			fprintf( fidtrend3, " ---------- ");
	fprintf( fidtrend3, " -------\t ");		
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)
			fprintf( fidtrend3, "----------- ");
	fprintf( fidtrend3, "---------\t ");		
	if (outletmodelflag == 1)
		for (p=0; p < maxnoutlet; p++)
			fprintf( fidtrend3, "------------- \t ");
	fprintf( fidtrend3, "\n");
}
fprintf( fidtrend3, "%d   \t %.1f   \t", yr, Qtotal/1e6);
if (outletmodelflag == 1)
	for (p=0; p < maxnoutlet; p++)   
		fprintf( fidtrend3, " %.2f   \t", Qtotaloutletannual[p]/1e6);
fprintf( fidtrend3, " %.2f\t     ", Qsannual/1e9);
if (outletmodelflag == 1)
	for (p=0; p < maxnoutlet; p++)   
		fprintf( fidtrend3, "%.3f    \t ", Qsannualoutlet[p]/1e9);
fprintf( fidtrend3, "%.2f  \t ", Qbedannual/1e9);
if (outletmodelflag == 1)
	for (p=0; p < maxnoutlet; p++)   
		fprintf( fidtrend3, "    %.3f  \t ", Qbedannualoutlet[p]/1e9);
fprintf( fidtrend3, "\n");		

return(err);
}	/* end of hydroprintannual.c */
