/*
 *  Hydroreadclimate.c
 *
 *
 *  Author:   A.J. Kettner (August-October 2002) (February 2003)
 *
 *
 * Variable		Def.Location		Type		Units	Usage
 * --------		------------		----		-----	-----
 * err			various				int			-		error flag, halts program
 * count		Read_Rainfall_Etc	int			-
 * dummyT		Read_Rainfall_Etc	double		deg.C
 * dummyTtot	Read_Rainfall_Etc	double		deg.C
 * dummyT2		Read_Rainfall_Etc	double		deg.C
 * dummyR		Read_Rainfall_Etc	double		mm
 * dummyRtot	Read_Rainfall_Etc	double		mm
 * HOURSINDAY	Read_Rainfall_Etc	-			hours	define the hours in a day
 * i			Read_Rainfall_Etc	int			-
 * k			Read_Rainfall_Etc	int			-
 * line[200]	Read_Rainfall_Etc	char		-
 * n			Read_Rainfall_Etc	long		-		counter for the number of years
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hydroalloc_mem.h"
#include "hydroinout.h"
#include "hydroparams.h"
#include "hydroreadclimate.h"
#include "hydrotimeser.h"
#include "hydroclimate.h"

/*-----------------------
 *  Function Definition
 *-----------------------*/
void Read_Rainfall_Etc(gw_rainfall_etc* gw_rain);

/*---------------------------
 *  Start of HydroReadInput
 *---------------------------*/
int hydroreadclimate(gw_rainfall_etc* gw_rain) {

/*-------------------
 *  Local Variables
 *-------------------*/
int k,err;

/*-----------------------
 *  Set local Variables
 *-----------------------*/
err=0;
ep=0;
total_yr=0;
for (k=0; k<nepochs; k++)
	total_yr += nyears[ep];
	
/*-------------------------------
 * Read Rainfall input FOR NOW
 *-------------------------------*/
    Read_Rainfall_Etc(gw_rain);
    return(err);
} /* end hydroreadclimate.c */

   
/*-----------------------
 *  Function Prototypes
 *-----------------------*/
  
/*------------------------------
 *  Function Read_Rainfall_Etc
 *------------------------------*/
void Read_Rainfall_Etc(gw_rainfall_etc* gw_rain ) {

/*-------------------
 *  Local Variables
 *-------------------*/
#define HOURSINDAY  (24.0) 
int k, count,i;
long n;
char line[200];
double dummyT, dummyR, dummyT2, dummyTtot, dummyRtot, dummydouble;

/*------------------------
 *  Open rain input file
 *------------------------*/
	if (webflag == 1){
		if ((fidinputgw_r = fopen(ffnameinputgw_r, "r")) == NULL){
    	   	fprintf(stderr, "  Read_Rainfall_Etc MESSAGE: Unable to open input file %s \n",ffnameinputgw_r);
        	fprintf(stderr, "    Hydrotrend will generate it's own climate values based on\n");
	   	    fprintf(stderr, "    line 14-25 of the input values in the input file.\n\n");
    	   	raindatafile = 0;
	    }
	    else raindatafile = 1;
	}
	if (webflag != 1){
	   	if ((fidinputgw_r = fopen(fnameinputgw_r, "r")) == NULL){
    	   	fprintf(stderr, "  Read_Rainfall_Etc MESSAGE: Unable to open input file %s \n",fnameinputgw_r);
        	fprintf(stderr, "    Hydrotrend will generate it's own climate values based on\n");
	   	    fprintf(stderr, "    line 14-25 of the input values in the input file.\n\n");
    	   	raindatafile = 0;
	    }
    	else raindatafile = 1;
	}
	if (raindatafile == 1){
		
/*--------------------
 *  Strip off header
 *--------------------*/
		for (k=0; k < 6; k++)
			fgets(line, sizeof(line), fidinputgw_r);

/*---------------------------------
 *  Read number of timesteps & dt
 *---------------------------------*/
		fgets(line, sizeof(line), fidinputgw_r);
		sscanf(line, "%ld %ld", &gw_rain->n_steps, &gw_rain->dt);

/*---------------------------------------------------
 *  Check number of years to run with actually rain
 *  data. If actually rain data is not equal with 
 *  the number of years to run, stop program.
 *---------------------------------------------------*/
		if (gw_rain->n_steps/daysiy < total_yr){
			dummydouble = gw_rain->n_steps/daysiy;
			fprintf(stderr, "  HydroReadclimate.c ERROR: Unable to run hydrotrend because of\n"); 
			fprintf(stderr, "    insufficient climate data\n");
			fprintf(stderr, "    Number of model years in HYDRO.IN, total years from all epochs: %d (line 5).\n",total_yr);
			fprintf(stderr, "    is not equal to number of years of climate data: %f.\n",dummydouble);
			fprintf(stderr, "    program aborted \n");
			exit(-1);
		}	

/*------------------------------
 *  Allocate memory for arrays
 *------------------------------*/
		gw_rain->R = malloc2d(total_yr, daysiy, double);
		gw_rain->T = malloc2d(total_yr, daysiy, double);
		gw_rain->Tperyear = malloc1d(total_yr, double);
 
/*----------------------------------------------
 *  Read Precipitation and Temperature values
 *  If values are not daily values 
 *  but for example hour values, then
 *  average the temperature and add
 *  all the precipitation values for that day.
 *----------------------------------------------*/
		for (n=0; n < total_yr; n++){
			dummyT2=0.0;
			for (k=0; k<daysiy; k++){
				dummyRtot = 0.0;
				dummyTtot = 0.0;
				for (i=0; i<(HOURSINDAY/gw_rain->dt); i++){
					fgets(line, sizeof(line), fidinputgw_r);
					count = sscanf(line, "%lf %lf",&dummyR, &dummyT);
					if (count != 2){
						n =(n+1)*(k+1)*(i+1);
						fprintf(stderr, "  HydroReadclimate.c ERROR: Error occured when\n" );
						fprintf(stderr, "    trying to read line %ld\n", n );
						fprintf(stderr, "    of file: %s.\n",fnameinputgw_r );
						fprintf(stderr, "    Precipitation or Temperature data is missing\n" );
				        fprintf(stderr, "    program aborted \n" );
        				exit(-1);
        			}
					if (dummyR < 0.0 || dummyR > 1000.0){
						fprintf(stderr, "  HydroReadclimate.c ERROR:\n" );
						fprintf(stderr, "    Precipitation data out of range,\n");
						fprintf(stderr, "    ppt is 1000.0 < %f < 0.0\n ",dummyR );
						fprintf(stderr, "    program aborted \n" );    	    	
						exit(-1);
					}
					if (dummyT < -50.0 || dummyT > 50.0){
						fprintf(stderr, "  HydroReadclimate.c ERROR:\n" );
						fprintf(stderr, "    Precipitation data out of range,\n");
						fprintf(stderr, "    ppt of 80.0 < %f < -80.0 ",dummyT );
						fprintf(stderr, "    program aborted \n" );    	    	
						exit(-1);
					}   	    	
					dummyRtot += dummyR;
					dummyTtot += dummyT;
				}
				dummyTtot = ((dummyTtot)/(HOURSINDAY/gw_rain->dt));
				gw_rain->R[n][k] = dummyRtot;
				gw_rain->T[n][k] = dummyTtot;
				gw_rain->R[n][k] *= (Pmassbal[ep]/1000.0);
				dummyT2 +=gw_rain->T[n][k];
			}
			gw_rain->Tperyear[n]=dummyT2/daysiy;
		}
/*--------------------
 *  Close input file
 *--------------------*/
		fclose(fidinputgw_r);
	}
    return;
}  /* end Read_Rainfall_Etc */   


