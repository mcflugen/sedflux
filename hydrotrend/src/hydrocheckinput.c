/*
 *  HydroCheckInput.c
 *
 *  Checks the input parameters for HYDROTREND
 *
 *  Author:    M.D. Morehead (June 1998)
 *  Author2:   S.D. Peckham  (January 2002)
 *  Author3:   A.J. Kettner  (August-September 2002)
 *
 */
#include <string.h>
#include "hydroclimate.h"
#include "hydroinout.h"
#include "hydroparams.h"

/*----------------------------
 *  Start of HydroCheckInput
 *----------------------------*/
int hydrocheckinput()
{
/*-------------------
 *  Local Variables
 *-------------------*/
double dumdbl;
int jj, err, verbose;
err = 0;
verbose = 0;

/*-------------------------------
 *  Verify the number of epochs
 *-------------------------------*/
if (verbose) printf("Checking number of epochs...\n");
if (nepochs <= 0 || nepochs >= maxepoch) {
    fprintf(stderr,"   HydroCheckInput ERROR: nepochs < 0 or > maxepoch in the input file.\n");
    fprintf(stderr,"      nepochs = %d \n", nepochs);
    err=1;
}

/*----------------------------------------------
 *  Loop through and test values of each epoch
 *----------------------------------------------*/
for (ep=0; ep<nepochs; ep++) {

/*----------------------------------------
 *  Check number of years for this epoch
 *----------------------------------------*/
if (verbose) printf("Checking number of years in epoch...\n");
if (nyears[ep] < 1 ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 5) \n");
    fprintf(stderr,"      nyears < 1 in the input file.\n");
    fprintf(stderr,"      nyears[ep] = %d \n", nyears[ep]);
    err++;
}
if (ep) {
    if( syear[ep-1]+nyears[ep-1] != syear[ep] ) {
        fprintf(stderr,"   HydroCheckInput ERROR: input file line 5) \n");
        fprintf(stderr,"      The end year of one epoch does not match the \n");
        fprintf(stderr,"      beginning year of the following epoch. \n");
        fprintf(stderr,"      epoch \t\t\t = %d \n", ep+1 );
        fprintf(stderr,"      syear[ep-1] \t\t = %d \n", syear[ep-1]);
        fprintf(stderr,"      nyears[ep-1] \t\t = %d \n", nyears[ep-1]);
        fprintf(stderr,"      endyear[ep-1] \t\t = %d \n", syear[ep-1]+nyears[ep-1]-1);
        fprintf(stderr,"      predicted syear[ep] \t = %d \n", syear[ep-1]+nyears[ep-1]);
        fprintf(stderr,"      syear[ep] \t\t = %d \n", syear[ep]);
        err++;
    }
}

/*-----------------------------------------------------------
 *  Check that the averaging timestep for the *.dis file is
 *  one of: d (day), m (month), s (season) or y (year).		
 *-----------------------------------------------------------*/
if (verbose) printf("Checking timestep...\n");
if( timestep[0] != 'd' && timestep[0] != 'm' &&
    timestep[0] != 's' && timestep[0] != 'y' ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 5 \n");
    fprintf(stderr,"      The averaging timestep is incorrect.\n");
    fprintf(stderr,"      It must be one of d (day), m (month), s (season), y (year).\n");
    fprintf(stderr,"      timestep = %s \n", timestep);
    err++;
}

/*--------------------------------------------------------------
 *  Ensure that table year ranges fall within epoch boundaries	
 *--------------------------------------------------------------*/
if (verbose) printf("Checking table year ranges...\n");
if (tblstart[ep] < syear[ep] || tblend[ep] < syear[ep] ||
    tblstart[ep] > syear[ep]+nyears[ep] || tblend[ep] > syear[ep]+nyears[ep] ) {
    fprintf(stderr,"   HydroCheckInput WARNING: input file line 6) \n");
    fprintf(stderr,"	 The specified years for the table file are out of range.\n");
    fprintf(stderr,"	 No information will be printed.\n");
    fprintf(stderr,"	 syear[ep] \t = %d \n", syear[ep]);
    fprintf(stderr,"	 nyears[ep] \t = %d \n", nyears[ep]);
    fprintf(stderr,"	 tblstart[ep] \t = %d \n", tblstart[ep]);
    fprintf(stderr,"	 tblend[ep] \t = %d \n", tblend[ep]);
}

/*-------------------------------
 *  Check number of grain sizes
 *-------------------------------*/
if (verbose) printf("Checking number of grain sizes...\n");
if (ngrain > maxgrn || ngrain < 0){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 7) \n");
    fprintf(stderr,"	 The number of grainsizes is not in the range \n");
    fprintf(stderr,"	 between 0 and %d.\n",maxgrn);
    err++;
}

/*-----------------------------------------------------------
 *  Check that grain size fractions sum to 1.0 (within .1%)	
 *-----------------------------------------------------------*/
if (verbose) printf("Checking grain fraction sum...\n");
dumdbl = 0.0;
for (jj=0; jj<ngrain; jj++ )
    dumdbl += grainpct[jj][ep];
if (fabs(1-dumdbl) > 0.001 ){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 8) \n");
    fprintf(stderr,"	 The sum of the grain size fractions does not = 1. \n");
    fprintf(stderr,"	 sum(grainpct[ep]) = %f \n", dumdbl);
    err++;
}

/*----------------------------------------------------
 *  Check that temp trend lines match between epochs
 *----------------------------------------------------*/
if (ep) {
   if (verbose) printf("Checking temp trend line continuity...\n");
   dumdbl = 1 - Tstart[ep]/ (Tstart[ep-1]+nyears[ep-1]*Tchange[ep-1]);
   if (fabs(dumdbl) > 0.01 ) {
       fprintf(stderr,"   HydroCheckInput ERROR: input file line 9) \n");
       fprintf(stderr,"	 The start temperature for one epoch does not match the end\n");
       fprintf(stderr,"	 temperature of the previous epoch.\n");
       fprintf(stderr,"	 Criteria: fabs(difference) < 0.01 \n");
       fprintf(stderr,"	 Criteria \t = %f \n", dumdbl);
       fprintf(stderr,"	 epoch \t\t = %d \n", ep+1 );
       fprintf(stderr,"	 Tstart[ep] \t = %f (degC) \n", Tstart[ep]);
       fprintf(stderr,"	 Tstart[ep-1] \t = %f (degC) \n", Tstart[ep-1]);
       fprintf(stderr,"	 Tchange[ep-1] \t = %f (degC/a) \n", Tchange[ep-1]);
       fprintf(stderr,"	 nyears[ep-1] \t = %d \n", nyears[ep-1]);
       err++;
	}

/*------------------------------------------------------
 *  Check that precip trend lines match between epochs
 *------------------------------------------------------*/
   if (verbose) printf("Checking precip trend line continuity...\n");
   dumdbl = 1 - Pstart[ep] / (Pstart[ep-1]+nyears[ep-1]*Pchange[ep-1]);
   if (fabs(dumdbl) > 0.01 ) {
       fprintf(stderr,"   HydroCheckInput ERROR: input file line 10) \n");
       fprintf(stderr,"	 The start precipitation for one epoch does not match the end\n");
       fprintf(stderr,"	 precipitation of the previous epoch.\n");
       fprintf(stderr,"	 Criteria: fabs(difference) < 0.01 \n");
       fprintf(stderr,"	 Criteria \t = %f \n", dumdbl);
       fprintf(stderr,"	 epoch \t\t = %d \n", ep+1 );
       fprintf(stderr,"	 Pstart[ep] \t = %f (m/a) \n",   Pstart[ep]);
       fprintf(stderr,"	 Pstart[ep-1] \t = %f (m/a) \n", Pstart[ep-1]);
       fprintf(stderr,"	 Pchange[ep-1] \t = %f (m/a/a) \n", Pchange[ep-1]);
       fprintf(stderr,"	 nyears[ep-1] \t = %d \n", nyears[ep-1]);
       err++;
    }
}

/*----------------------------------------------
 *  Check the ranges of annual temp and precip
 *----------------------------------------------*/
if (verbose) printf("Checking temp and precip trend parameters...\n");
if( -20. > Tstart[ep] || Tstart[ep] > 30. ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 9) \n");
    fprintf(stderr,"	 The starting temperature is out of range. \n");
    fprintf(stderr,"	 Criteria: -20 < Tstart < 30 (deg C) \n");
    fprintf(stderr,"	 Tstart[ep] = %f \n", Tstart[ep]);
    err++;
}
if( -1 > Tchange[ep] || Tchange[ep] > 1 ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 9) \n");
    fprintf(stderr,"	 The temperature change/year is out of range. \n");
    fprintf(stderr,"	 Criteria: -1 < Tchange < 1 (degC/a) \n");
    fprintf(stderr,"	 Tchange[ep] = %f \n", Tchange[ep]);
    err++;
}
if( -5 > Tstd[ep] || Tstd[ep] > 5 ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 9) \n");
    fprintf(stderr,"	 The temperature standard deviation is out of range. \n");
    fprintf(stderr,"	 Criteria: -5 < Tstd < 5 (degC) \n");
    fprintf(stderr,"	 Tstd[ep] = %f \n", Tstd[ep]);
    err++;
}
if( 0 > Pstart[ep] || Pstart[ep] > 5 ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 10) \n");
    fprintf(stderr,"	 The starting precipitation is out of range. \n");
    fprintf(stderr,"	 Criteria: 0 < Pstart < 5 (m/a) \n");
    fprintf(stderr,"	 Pstart[ep] = %f \n", Pstart[ep]);
    err++;
}
if( -0.5 > Pchange[ep] || Pchange[ep] > 0.5 ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 10) \n");
    fprintf(stderr,"	 The precipitation change/year is out of range. \n");
    fprintf(stderr,"	 Criteria: -0.5 < Pchange < 0.5 (m/a/a) \n");
    fprintf(stderr,"	 Pchange[ep] = %f \n", Pchange[ep]);
    err++;
}
if( -2 > Pstd[ep] || Pstd[ep] > 2 ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 10) \n");
    fprintf(stderr,"	 The precipitation standard deviation is out of range. \n");
    fprintf(stderr,"	 Criteria: -2 < Pstd < 2 (m/a) \n");
    fprintf(stderr,"	 Pstd[ep] = %f \n", Pstd[ep]);
    err++;
}

/*------------------------------------------------------------
 *  Check the range of the monthly climate variable (T P...)
 *  Check that some rain is specified (in any month)
 *------------------------------------------------------------*/
if (verbose) printf("Checking monthly climate data (T & P)...\n");
dumdbl = 0.0;
for (jj=0; jj<12; jj++) {
    if( -50 > Tnominal[jj][ep] || Tnominal[jj][ep] > 50 ) {
        fprintf(stderr,"   HydroCheckInput ERROR: input file line %d) \n", jj+13);
        fprintf(stderr,"	 The average monthly temperature is out of range. \n");
        fprintf(stderr,"	 Criteria: -50 < Tnominal < 50 \n");
        fprintf(stderr,"	 Tnominal[jj][ep] \t = %f (degC) \n", Tnominal[jj][ep]);
        fprintf(stderr,"	 jj \t\t = %d \n", jj);
        fprintf(stderr,"	 ep \t\t = %d \n", ep);
        err++;
    }
    if( 0.0 > Tnomstd[jj][ep] || Tnomstd[jj][ep] > 10 ) {
        fprintf(stderr,"   HydroCheckInput ERROR: input file line %d) \n", jj+13);
        fprintf(stderr,"	 The monthly temperature standard deviation is out of range. \n");
        fprintf(stderr,"	 Criteria: 0.0 < Tnomstd < 10 \n");
        fprintf(stderr,"	 Tnomstd[jj][ep] \t = %f (degC) \n", Tnomstd[jj][ep]);
        fprintf(stderr,"	 jj \t\t = %d \n", jj);
        fprintf(stderr,"	 ep \t\t = %d \n", ep);
        err++;
    }
    if( 0. > Pnominal[jj][ep]*1000 || Pnominal[jj][ep]*1000 > 1000 ) {
        fprintf(stderr,"   HydroCheckInput ERROR: input file line %d) \n", jj+13);
        fprintf(stderr,"	 The monthly rainfall is out of range. \n");
        fprintf(stderr,"	 Criteria: 0 < Pnominal < 1000 \n");
        fprintf(stderr,"	 Pnominal[jj][ep] \t = %f (mm) \n", Pnominal[jj][ep]*1000);
        fprintf(stderr,"	 jj \t\t = %d \n", jj);
        fprintf(stderr,"	 ep \t\t = %d \n", ep);
        err++;
    }
    if( 0 >= Pnomstd[jj][ep]*1000 || Pnomstd[jj][ep]*1000 > 650 ) {
        fprintf(stderr,"   HydroCheckInput ERROR: input file line %d) \n", jj+13);
        fprintf(stderr,"	 The monthly rainfall standard deviation is out of range. \n");
        fprintf(stderr,"	 Criteria: 0 < Pnomstd < 450 \n");
        fprintf(stderr,"	 Pnomstd[jj][ep] \t = %f (mm) \n", Pnomstd[jj][ep]*1000);
        fprintf(stderr,"	 jj \t\t = %d \n", jj);
        fprintf(stderr,"	 ep \t\t = %d \n", ep);
        err++;
    }
    dumdbl+=Pnominal[jj][ep];
}
if( dumdbl <= 0 ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file lines 13-24) \n");
    fprintf(stderr,"	 There is no precipitation specified in any month. \n");
    fprintf(stderr,"	 Total specified rainfall = %f (m) \n", dumdbl*1000);
    err++;
}

/*-----------------------------------------------------------
 *  Check the bounds on the lapse rate	
 *  psuedoAdiabatic Lapse rate 6 degC/km in mid troposphere
 *  gamma_s = saturate adiabatic lapse rate == - dT/dz
 *  = 4 degC/km near the ground in humid air masses
 *  = 6-7 degC/km in the middle troposphere
 *-----------------------------------------------------------*/
if (verbose) printf("Checking lapse rate...\n");
if ((1 > lapserate[ep]*1000 && lapserate[ep] != -9999) || lapserate[ep]*1000 > 10) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 25) \n");
    fprintf(stderr,"	 The adiabatic lapse rate is out of range. \n");
    fprintf(stderr,"	 Criteria: 1 < lapserate < 10 \n");
    fprintf(stderr,"	 lapserate[ep] = %f (degC/km) \n", lapserate[ep]*1000);
    err++;
}

/*------------------------------
 *  Check latitude input value
 *------------------------------*/
if (verbose) printf("Checking latitude...\n");
if (lat > 90 || lat < -90){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 38) \n");
    fprintf(stderr,"	 Latitude input is out of range, lower than -90 or  \n");
    fprintf(stderr,"	 higher than 90 degrees. lat = %f (degrees)\n", lat);
    err++;
}

/*----------------------------------
 *  Check ascii file ON/OFF option
 *----------------------------------*/
if (verbose) printf("Checking ascii file ON/OFF option...\n");
if (ep == 0){
    if( (strncmp(asciioutput,ON,2) != 0) && ( strncmp(asciioutput,OFF,2) != 0)){
        fprintf(stderr,"    HydroCheckInput ERROR: input file line 2) \n");
        fprintf(stderr,"      The print to ASCII file option is not set  \n");
        fprintf(stderr,"      correctly to ON or OFF, but is set as: ");
        for (jj=0; jj<MAXCHAR; jj++)
            fprintf(stderr,"%c",asciioutput[jj]);
        fprintf(stderr,"\n");
        err++;
    }
}

/*--------------------------------------------------
 *  Check ELA range and values at epoch boundaries
 *--------------------------------------------------*/
if (verbose) printf("Checking ELA info...\n");
if( 0 > ELAstart[ep] || 0 > ELAstart[ep]+nyears[ep]*ELAchange[ep] ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 26) \n");
    fprintf(stderr,"	 The ela is out of range. \n");
    fprintf(stderr,"	 Criteria: 0 < ela \n");
    fprintf(stderr,"	 ELAstart[ep] \t = %f (m) \n", ELAstart[ep]);
    fprintf(stderr,"	 ela stop \t = %f (m) \n", ELAstart[ep]+nyears[ep]*ELAchange[ep]);
    err++;
}
if( ep ) {
    dumdbl = 1 - ELAstart[ep] / (ELAstart[ep-1]+nyears[ep-1]*ELAchange[ep-1]);
    if( fabs(dumdbl) > 0.01 ) {
        fprintf(stderr,"   HydroCheckInput ERROR: input file line 26) \n");
        fprintf(stderr,"      The ela at the end of one epoch does not match the \n");
        fprintf(stderr,"      ela at the begining of the following epoch. \n");
        fprintf(stderr,"      Criteria: fabs(differnce) < 0.01 \n");
        fprintf(stderr,"      Criteria \t = %f \n", dumdbl);
        fprintf(stderr,"      epoch-1 \t = %d \n", ep);
        fprintf(stderr,"      ELAstart[ep-1] \t = %f (m) \n", ELAstart[ep-1] );
        fprintf(stderr,"      ELAchange[ep-1] \t = %f (m/a) \n",ELAchange[ep-1] );
        fprintf(stderr,"      epoch \t = %d \n", ep+1);
        fprintf(stderr,"      ELAstart[ep] \t = %f (m) \n", ELAstart[ep]);
        fprintf(stderr,"      ela end [ep-1] \t = %f (m) \n",ELAstart[ep-1]+nyears[ep-1]*ELAchange[ep-1] );
        err++;
    }
}

/*---------------------------------------------------
 *  Check nival and ice groundwater and evaporation
 *  ground water fraction is 0 to .5	
 *---------------------------------------------------*/
if (verbose) printf("Checking snow and ice parameters...\n");
if (0 > dryevap[ep] || dryevap[ep] > 0.9){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 27) \n");
    fprintf(stderr,"	 The snow and ice evaporation percentage is out of range. \n");
    fprintf(stderr,"	 0 < dryevap < 0.9 \n");
    fprintf(stderr,"	 dryevap[ep] = %f (%%) \n", dryevap[ep]);
    err++;
}

/*----------------------------
 *  Check river slope limits	
 *----------------------------*/
if (verbose) printf("Checking river slope near mouth...\n");
if (0.00000001 > rslope[ep] || rslope[ep] > 0.1 ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 28) \n");
    fprintf(stderr,"	 The river slope is out of range. \n");
    fprintf(stderr,"	 Criteria: 0.001 < rslope < 0.01 \n");
    fprintf(stderr,"	 rslope[ep] = %f (m/m (gradient)) \n", rslope[ep]);
    err++;
}

/*----------------------------
 *  Check river basin length
 *----------------------------*/
if (verbose) printf("Checking basin length...\n");
if( 10 > basinlength[ep]/1000 || basinlength[ep]/1000 > 10000 ){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 29) \n");
    fprintf(stderr,"	 The basin length is out of range. \n");
    fprintf(stderr,"	 10 < basinlength < 1000 \n");
    fprintf(stderr,"	 basinlength[ep] = %f (km) \n", basinlength[ep]/1000);
    err++;
}

/*-----------------------------
 *  Check volume of reservoir
 *-----------------------------*/
if (verbose) printf("Checking volume of reservoir...\n");
if(  0 > Rvol[ep] || Rvol[ep] > 8000.0){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 30) \n");
    fprintf(stderr,"	 The volume of reservoirs is out of range. \n");
    fprintf(stderr,"	 0.0 km^3 < Rvol[ep]< 8000.0 km^3 \n");
    fprintf(stderr,"     Rvol[ep] = 0 if there are no large reservoirs\n");
    fprintf(stderr,"	 Rvol[ep] = %.3f (km^3) \n", Rvol[ep]);
    err++;
}
/*-------------------------------
 *  Check altiture of reservoir
 *-------------------------------*/
if (verbose) printf("Checking altitude of reservoir...\n");
if (Rvol[ep] != 0.0)
	if (Ralt[ep] < 0.0 || Ralt[ep] >= maxalt[ep]){
	    fprintf(stderr,"   HydroCheckInput ERROR: input file line 30) \n");
    	fprintf(stderr,"	 The altitude of reservoirs is out of range. \n");
	    fprintf(stderr,"	  0 > Ralt[ep] = %.3f (km^3) > maxalt[ep] = %f \n", Ralt[ep], maxalt[ep]);
    	err++;
	}

/*-------------------------------------------------
 *  Check hydraulic geometry coeffs and exponents
 *-------------------------------------------------*/
if (verbose) printf("Checking hydraulic geometry parameters...\n");
if ((0 >= velpow[ep] && velpow[ep] != -9999) || velpow[ep] >= 1) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 31) \n");
    fprintf(stderr,"	 The river mouth velocity power is out of range. \n");
    fprintf(stderr,"	 Criteria: 0 < velpow < 1 \n");
    fprintf(stderr,"	 velpow[ep] = %f \n", velpow[ep]);
    err++;
}
if( 0 >= velcof[ep] ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 31) \n");
    fprintf(stderr,"	 The river mouth velocity coefficient is out of range. \n");
    fprintf(stderr,"	 Criteria: 0 < velcof \n");
    fprintf(stderr,"	 velcof[ep] = %f \n", velcof[ep]);
    err++;
}
if( (0 >= widpow[ep] && widpow[ep] != -9999) || widpow[ep] >= 1 ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 32) \n");
    fprintf(stderr,"	 The river mouth width power is out of range. \n");
    fprintf(stderr,"	 Criteria: 0 < widpow < 1 \n");
    fprintf(stderr,"	 widpow[ep] = %f \n", widpow[ep]);
    err++;
}
if( 0 >= widcof[ep] ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 32) \n");
    fprintf(stderr,"	 The river mouth width coefficient is out of range. \n");
    fprintf(stderr,"	 Criteria: 0 < widcof \n");
    fprintf(stderr,"	 widcof[ep] = %f \n", widcof[ep]);
    err++;
}

/*--------------------------------
 *  Check average river velocity
 *--------------------------------*/
if (verbose) printf("Checking average river velocity...\n");
if( 0.1 > avgvel[ep] || avgvel[ep] > 5 ){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 33) \n");
    fprintf(stderr,"	 The average river velocity is out of range. \n");
    fprintf(stderr,"	 0.1 < avgvel < 5 \n");
    fprintf(stderr,"	 avgvel[ep] = %f (m/s) \n", avgvel[ep]);
    err++;
}

/*----------------------------------------
 *  Check Groundwater Storage parameters		
 *----------------------------------------*/
if (verbose) printf("Checking groundwater parameters...\n");
if( 2 > gwmax[ep] || gwmax[ep] > 1e15 ){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 34) \n");
    fprintf(stderr,"	 The maximum size of the groundwater pool is out of range. \n");
    fprintf(stderr,"	 2 < gwmax < 1e15 \n");
    fprintf(stderr,"	 gwmax[ep] = %f (m^3) \n", gwmax[ep]);
    err++;
}
if( 1 > gwmin[ep] || gwmin[ep] > gwmax[ep] ){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 34) \n");
    fprintf(stderr,"	 The minimum size of the groundwater pool is out of range. \n");
    fprintf(stderr,"	 1 < gwmin < gwmax \n");
    fprintf(stderr,"	 gwmin[ep] = %f (m^3) \n", gwmin[ep]);
    fprintf(stderr,"	 gwmax[ep] = %f (m^3) \n", gwmax[ep]);
    err++;
}
if( gwmin[ep] > gwinitial || gwinitial > gwmax[ep] ){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 35) \n");
    fprintf(stderr,"	 The initial size of the groundwater pool is out of range. \n");
    fprintf(stderr,"	 gwmin < gwinitial < gwmax \n");
    fprintf(stderr,"	 gwmin[ep]  = %f (m^3) \n", gwmin[ep]);
    fprintf(stderr,"	 gwinitial  = %f (m^3) \n", gwinitial);
    fprintf(stderr,"	 gwmax[ep]  = %f (m^3) \n", gwmax[ep]);
    err++;
}

/*------------------------------------------------------
 *  Check ranges on the rain mass balance coefficients
 *------------------------------------------------------*/
if (verbose) printf("Checking mass balance coeffs...\n");
if( 0 > Pmassbal[ep] || Pmassbal[ep] > 10 ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 11) \n");
    fprintf(stderr,"	 The rain mass balance is out of range. \n");
    fprintf(stderr,"	 Criteria: 0 < Pmassbal < 10  \n");
    fprintf(stderr,"	 Pmassbal[ep] = %f \n", Pmassbal[ep]);
    err++;
}
if( 1 >= Pexponent[ep] || Pexponent[ep] >= 2 ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 11) \n");
    fprintf(stderr,"	 The rain distribution exponent is out of range. \n");
    fprintf(stderr,"	 Criteria: 1 < Pexponent < 2 \n");
    fprintf(stderr,"	 Pexponent[ep] = %f \n", Pexponent[ep]);
    err++;
}
if( 0 > Prange[ep] || Prange[ep] > 10 ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 11) \n");
    fprintf(stderr,"	 The rain mass balance distribution range is out of range. \n");
    fprintf(stderr,"	 Criteria: 0 < Prange < 10 \n");
    fprintf(stderr,"	 Prange[ep] = %f \n", Prange[ep]);
    err++;
}

/*----------------------------------------------------------
 *  Check that baseflow is >= 0 and less than total precip
 *----------------------------------------------------------*/
if (verbose) printf("Checking baseflow range...\n");
if( 0. > baseflowtot[ep] || baseflowtot[ep] > Pstart[ep]*totalarea[ep]/(dTOs*365.0) ) {
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 12) \n");
    fprintf(stderr,"	 The baseflow is out of range. \n");
    fprintf(stderr,"	 Criteria: 0.0 < baseflow < total Precip (m^3/s) \n");
    fprintf(stderr,"	 baseflow[ep] = %f (m^3/s) \n", baseflowtot[ep]);
    fprintf(stderr,"	 total Precip = Pstart*totalarea/(dTOs*365.0) \n");
    fprintf(stderr,"	 total Precip = %f (m^3/s) \n", Pstart[ep]*totalarea[ep]/(dTOs*365.0) );
    fprintf(stderr,"	 Pstart[ep] = %f (m) \n", Pstart[ep] );
    fprintf(stderr,"	 totalarea[ep] = %f (km^2) \n", totalarea[ep]/1e6 );
    err++;
}

/*----------------------------------------------
 *  50) Check Subsurface Storm Flow parameters	
 *----------------------------------------------*/
if (verbose) printf("Checking storm flow parameters...\n");
if (1 > alphass[ep] || alphass[ep] > 1e5){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 36) \n");
    fprintf(stderr,"	 The subsurface storm flow coefficient is out of range. \n");
    fprintf(stderr,"	 1 < alphass < 1e5 \n");
    fprintf(stderr,"	 alphass[ep] = %f (m^3/s) \n", alphass[ep]);
    err++;
}
if( 0.5 > betass[ep] || betass[ep] > 2 ){	/* betass 0:10 in river5 */
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 36) \n");
    fprintf(stderr,"	 The subsurface storm flow exponent is out of range. \n");
    fprintf(stderr,"	 0.5 < betass < 2 \n");
    fprintf(stderr,"	 betass[ep] = %f (-) \n", betass[ep]);
    err++;
}

/*---------------------------------
 *  Check infiltration parameters	
 *---------------------------------*/
if (verbose) printf("Checking infiltration parameters...\n");
if( 10 > Ko[ep]*1000 || Ko[ep]*1000 > pmax[ep]*1000 ){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 37) \n");
    fprintf(stderr,"	 The saturated hydraulic conductivity is out of range. \n");
    fprintf(stderr,"	 10 < Ko < pmax \n");
    fprintf(stderr,"	 Ko[ep] = %f (mm/day) \n", Ko[ep]*1000);
    fprintf(stderr,"	 pmax[ep] = %f (mm/day) \n", pmax[ep]*1000);
    err++;
}

/*---------------------------
 *  Check number of outlets
 *---------------------------*/
if (noutletflag != 1){
	if (verbose) printf("Checking number of outlets...\n");
	if ((noutlet > maxnoutlet || noutlet < 0) ){
		fprintf(stderr,"   HydroCheckInput ERROR: input file line 40) \n");
		fprintf(stderr,"	 The number of outlets is not in the range \n");
		fprintf(stderr,"	 between 0 and 10. The filled out nr. = %d.\n",noutlet);
		err++;
	}
}

/*if (noutletflag == 1){
	if (verbose) printf("Checking number of outlets...\n");
	if (noutlet == != 'r' && noutletc != 'R' && noutletc != 'u' && noutletc != 'U'){
		fprintf(stderr,"   HydroCheckInput ERROR: input file line 40) \n");
		fprintf(stderr,"	 Hydrotrend doesn't have the right information to \n");
		fprintf(stderr,"	 determine the number of outlets. Value should be \n");
		fprintf(stderr,"	 'u' / 'U' for unknown or 'r' / 'R' for a given range \n");
		fprintf(stderr,"	 The filled out character is: %c",noutletc );
		err++;
	}
	if (noutletc == 'r' || noutletc == 'R' ){
		if (verbose) printf("Checking range of outlet numbers...\n");
		if (minnoutlet < 0 || maxnoutlet > 10 || minnoutlet >= maxnoutlet){
			fprintf(stderr,"   HydroCheckInput ERROR: input file line 40) \n");
			fprintf(stderr,"	 The minimum or maximum number of outlets is not in range \n");
			fprintf(stderr,"	 between 1 (min) and 10 (max). The filled out min =%d and max =%d\n",minnoutlet, maxnoutlet);
			err++;
		}
	}		
}*/
	
/*-------------------------------------------------------
 *  Check that outlet fractions sum to 1.0 (within .1%)	
 *-------------------------------------------------------*/
/*
 if ( nooutletpctflag == 0 ){
	if (verbose) printf("Checking outlet fraction sum...\n");
	dumdbl = 0.0;
	for (jj=0; jj<noutlet; jj++ )
    	dumdbl += outletpctdummy[ep][jj];
	if (fabs(1-dumdbl) > 0.001 ){
		fprintf(stderr,"   HydroCheckInput ERROR: input file line 41) \n");
		fprintf(stderr,"	 The sum of the outlet fractions does not = 1. \n");
		fprintf(stderr,"	 sum(outletpct[ep]) = %f \n", dumdbl);
		err++;
	}
 }
*/
/*-----------------------------------------------------------------------
 *  Check that sediment filter percentage is not out of range (0 - 0.9)	
 *-----------------------------------------------------------------------*/
if (verbose) printf("Checking filter fraction ...\n");
if (sedfilter[ep]> 0.9 || sedfilter[ep] < 0.0){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 42) \n");
    fprintf(stderr,"	 The sediment filter percentage is out of range \n");
    fprintf(stderr,"	 0.0 > %f > 0.9 \n", sedfilter[ep]);
    err++;
}

/*---------------------------------------------------------
 *  Check that formula flag for Qsbar is set within range	
 *---------------------------------------------------------*/
if (verbose) printf("Checking Qsbar formula flag ...\n");
if (Qsbarformulaflag[ep] < 0 || Qsbarformulaflag[ep] > 1){
    fprintf(stderr,"   HydroCheckInput ERROR: input file line 43) \n");
    fprintf(stderr,"	 Hydrotrend doesn't know which formula to use to calculate \n");
    fprintf(stderr,"	 Qsbar. Based on Area = 1, Based on Discharge = 0\n");
    fprintf(stderr,"     You set the input value to %d\n",Qsbarformulaflag[ep]);
    err++;
}


/*--------------------------------------------
 *  Write out epoch number if errors occured
 *--------------------------------------------*/
if (err > 0) {
   fprintf(stderr,"  The above errors occured in epoch %d \n",ep+1);
   fprintf(stderr,"  ------------------------------------ \n");
   ep = nepochs;
   }
} /* end of epoch loop */

/*-------------------------------------------------
 *  Write out general messages if errors occurred
 *-------------------------------------------------*/
if (err > 0) {
   fprintf(stderr,"  %d errors occured in HydroCheckInput \n", err);
   fprintf(stderr,"  Many of the limits are physically imposed. \n");
   fprintf(stderr,"  Some are artificially imposed to limit typos. \n");
   fprintf(stderr,"  Be careful changing the limits in HydroCheckInput.c \n");
   fprintf(stderr,"  ------------------------------------------- \n");
}
else 
    if (verbose) {
        printf("No errors found in input values. \n");
        printf("\n");
    }

return(err);
}  /* end of HydroCheckInput */

