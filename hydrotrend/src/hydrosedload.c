/*
 *	HydroSedLoad.c
 *
 *		Author1:	S.D. Peckham	(September 2001)	(January 2002)
 *      Author2:	A.J. Kettner	(August - September 2002)(february 2003)
 *
 *	Calculates the sediment load (suspended and bedload) discharging
 *	from the river mouth for each day, using new formula from Morehead
 *	et al.(2001).  The formula is:
 *
 *	Qs(i) = Psi * Qsbar * (Q/Qbar)^C
 *
 *    Qsbar = long-term average of Qs (for an epoch)
 *    Qsbar = alpha3 * (1-TE) * pow(A,alpha4) * pow(H,alpha5) * exp(k * Tbar)
 * or:
 *    Qsbar = alpha6 * (1-TE) * pow(Qbar,alpha7) * pow(H,alpha8) * exp(k * Tbar)
 * 
 *    Qbar  = long-term average of Q  (for an epoch)
 *    Tbar  = Tmean - ((lapserate[ep] * maxalt[ep])/3.0);
 *    Tmean = (Tstart[ep] + (Tstart[ep] + (Tchange[ep]*nyears[ep])))/2;
 *
 *    Psi   = a random number from a lognormal distribution with
 *            mean 1 and sigma = 0.763 * (0.99995^Qbar)
 *    C     = a random number from a distribution with mean cbar
 *            and standard deviation s, where:
 *            cbar = (1.4 - (0.025*T) + (0.00013*H) + (0.145*ln(Qsbar))
 *            and s = 0.17 + (0.0000183 * Qbar)
 *
 *	i  = subscript for instantaneous or daily values
 *    A  = totalarea = basin area (km^2)
 *    H  = maxalt = basin relief (m)
 *    T  = Tbar = mean basin temp (C)
 *	Q  = daily discharge (m^3/s)
 *	Qs = daily sediment flux (kg/s)
 *	Cs = daily sediment concentration (kg sediment/ m^3 water)
 * 
 * If reservoir capacity is larger than 0.5km^3:
 * Trapping efficiency (TE) calculated, based on paper of:
 * Charles J. Vorosmarty, Michel Meybeck, Balazs Rekete & Keshav Sharma: 
 * The potential impact of neo-Castorization on sediment transport by the 
 * global network of rivers (1997) in Human Impact on erosion and 
 * Sedimentation (Proceedings of the Rabat Symposium April 1997).
 * Based on the Brune equation:
 *
 *    TE =  Is only effective as lakes of reservoirs are turned on in 
 *          the input file.
 *    TEbasin = 1.0 - (0.05 / exp(Rvol/RQbar)0.5
 * 		Rvol = Volume of the reservoir
 * 		RQbar= discharge at the basin mouth of the reservoir 
 * 
 * If reservoir capacity is smaller than 0.5km^3:
 * Trapping efficiency (TE) calculated, based on paper of:
 * Gert Verstraeten and Jean Poesen:
 * Estimating trap efficiency of small reservoirs and ponds: methods and 
 * implications for the assessment of sediment yield.
 * Progress in Physical Geography 24,2 (2000) pp.219-251
 * 
 * TEbasin = ( 1.0 - (1.0 / (1 + 0.00021 * ((Rvol[ep] * 1e9) / Rarea[ep]))))
 * 
 */

#include <math.h>
#include <stdlib.h>
#include "hydroclimate.h"
#include "hydroinout.h"
#include "hydroparams.h"
#include "hydrotimeser.h"
#include "hydroreadclimate.h"
#include "hydroalloc_mem.h"
#include "hydrofree_mem.h"

#ifdef DBG
#include "hydroinout.h"
#endif

/*-------------------------
 *  Start of HydroSedLoad
 *-------------------------*/
int hydrosedload ( gw_rainfall_etc* gw_rain )
{

/*-------------------
 *  Local Variables
 *-------------------*/
#ifdef DBG
FILE *fid;
#endif

int err, i,p,kk,y;
long j;
double A, H, RQbar[maxepoch];
double Tbar, Tmean,Tend, Tdummy;
double Psi[daysiy], mu, sigma;
double cbar, s;
double ratio;
double unit_normal, normal;
double unit_normal2, normal2;
double trnfac;
double test;
double *Coutletannual, *cbaroutlet, *soutlet, **Coutlet, *sigmaoutlet;


/*------------------------
 *  Initialize Variables
 *------------------------*/
err  = 0;
y	= 0;
A    = (totalarea[ep] / 1e6);   /****  FORMULA USES AREA in km^2  ****/
H    = maxalt[ep];

/*-------------------------------------------------------
 *  Allocate memory for possible multiple outlet module
 *-------------------------------------------------------*/
	sigmaoutlet		= malloc1d( maxnoutlet, double );
	soutlet			= malloc1d( maxnoutlet, double );
	Coutlet			= malloc2d( daysiy, maxnoutlet, double );
	cbaroutlet		= malloc1d( maxnoutlet, double );
	Coutletannual	= malloc1d( maxnoutlet, double );
	Qsbartotoutlet	= malloc2d( maxepoch, maxnoutlet, double );

/*-------------------------------------
 *  Calculate Tbar for drainage basin
 *-------------------------------------*/
if ( raindatafile == 1 ){
	Tdummy=0.0;
	for (i=0; i<nyears[ep]; i++)
		Tdummy +=gw_rain->Tperyear[i];
	Tmean=Tdummy/nyears[ep];
}
else{
	Tend = Tstart[ep] + (Tchange[ep]*nyears[ep]);
	Tmean=(Tstart[ep] + Tend)/2;
}	
Tbar = Tmean - ( (lapserate[ep] * maxalt[ep]) / 3.0 );

/*-------------------------------------------------------
 *  Calculate trapping efficiency (TE) of the reservoir
 *-------------------------------------------------------*/
if (setstartmeanQandQs == 3 && yr == syear[ep]){
	if ( Rvol[ep] != 0.0 ){
		if ( Rarea[ep] == 0.0 )
			for (kk=0; kk<nhypts[ep];kk++)
				if ( hypselev[ep][kk] == Ralt[ep] || hypselev[ep][kk] > Ralt[ep] ){
					Rarea[ep] = A - ( hypsarea[ep][kk] / 1e6 );
					kk = nhypts[ep];
				}
			
		if (Rvol[ep] < 0.5){
	/*-------------------------------------
	 *  TE Calculated with Browns methode 
	 *-------------------------------------*/ 			
			TEsubbasin[ep] = ( 1.0 - (1.0 / (1 + 0.00021 * ((Rvol[ep] * 1e9) / Rarea[ep]))));
			TE[ep] = (Rarea[ep] / A) * TEsubbasin[ep];
		}
		
		else if (Rvol[ep] >= 0.5) {
	/*----------------------------------------------------
	 *  TE Calculated with Charles J. Vorosmarty methode 
	 *----------------------------------------------------*/ 					
		RQbar[ep] = Qbartotal[ep] * (Rarea[ep] / A);
		RQbar[ep] *=0.031536;  /* FORMULA USES QBAR in KM3/YEAR */
		TEsubbasin[ep] = 1.0 - (0.05 / pow(((Rvol[ep]/RQbar[ep])),0.5));
		TE[ep] = (Rarea[ep] / A) * TEsubbasin[ep];
		}
	}
	else if (Rvol[ep] == 0.0)
		TE[ep] = 0.0;
}

/*--------------------------------
 *  Compute Qsbar for this epoch
 *--------------------------------*/
if (Qsbarformulaflag[ep] == 1)
	if (setstartmeanQandQs == 3 && yr == syear[ep]){
		Qsbartot[ep] = alpha3 * (1.0 - TE[ep]) * pow(A,alpha4) * pow(H,alpha5) * exp(k1*Tbar);
	}
if (Qsbarformulaflag[ep] == 0){
	if (setstartmeanQandQs == 3 && yr == syear[ep])
		Qsbartot[ep] = (alpha6  *(1.0 - TE[ep]) * pow(Qbartotal[ep],alpha7) * pow(H,alpha8) * exp(k2*Tbar));
}
if (setstartmeanQandQs == 2)
	Qsbartot[ep] = 1;
/*-------------------------------------------
 *  Compute Qsbar per outlet for this epoch
 *-------------------------------------------*/
if (outletmodelflag == 1 && setstartmeanQandQs > 2){
	Qsoutletdummy = 0.0;
	for (p=0; p<maxnoutlet; p++){
  		Qsbartotoutlet[ep][p]= pow(Qbar[ep][p][eventcounter-eventsperyear],alpha7) ;
		Qsoutletdummy += Qsbartotoutlet[ep][p];
	}
}

/*--------------------------------------------
 *	Compute the bedload transport factor
 *--------------------------------------------*/
trnfac = ( rhowater[ep] * rhosed[ep] * trneff ) / 	\
	( (rhosed[ep] - rhowater[ep]) * tan( anglerep*degTOr ));

/*----------------------------------------------
 *  Get parameters for random numbers Psi & C.
 *  Qsum is passed in.
 *----------------------------------------------*/
mu    = 0.0;
s     = 0.17 + (0.0000183 * Qbartotal[ep]);
sigma = 0.763 * pow(0.99995, Qbartotal[ep]);
cbar  = 1.4 - (0.025 * Tbar) + (0.00013 * H) + (0.145 *log(Qsbartot[ep]));

/*------------------------------------------
 *  Initialize the annual values to zero.
 *  The Cs[i], Qs[i], and Qb[i] arrays are
 *  initialized to zero in HydroTrend.c.
 *------------------------------------------*/
Qsannual   = 0.0;
Csannual   = 0.0;
Qbedannual = 0.0;
if (outletmodelflag == 1 && setstartmeanQandQs > 2)
	for (p=0; p<maxnoutlet; p++){
		Qsannualoutlet[p]   = 0.0;
		Csannualoutlet[p]	= 0.0;
		Qbedannualoutlet[p] = 0.0;
		Coutletannual[p]	= 0.0;
	}	
if ( yr == syear[ep] ){
	Qsgrandtotal[ep] = 0.0;
	Csgrandtotal[ep] = 0.0;
	if ( outletmodelflag == 1 && setstartmeanQandQs > 2 )	
		for (p=0; p<maxnoutlet; p++){
			Qsgrandtotaloutlet[ep][p] = 0.0;
			Csgrandtotaloutlet[ep][p] = 0.0;
		}
}

/*--------------------------------------
 *  Loop through each day of year and
 *  calculate total sediment discharge.
 *  (daysiy defined in hydrotimeser.h)
 *--------------------------------------*/
for (i=0; i < daysiy; i++) {
    /*-----------------------------------
     *  Generate the random number, Psi
     *-----------------------------------*/
	unit_normal = ranarray[i];
    normal = (sigma * unit_normal) + mu;
    Psi[i] = exp(normal);  
}
    /*-----------------------------------
     *  Generate the random number, C
     *  (Can be normal or uniform.)
     *-----------------------------------*/
if (setstartmeanQandQs == 2 && yr == syear[ep]){	
	C = malloc1d( nyears[ep], double );
	for (j=0; j < nyears[ep]; j++) {
    	unit_normal2 = ((rand()/327680000.9));
//    	printf( "%f\n",unit_normal2);
	    normal2 = (s * unit_normal2) + cbar;
    	C[j] = normal2;
//    	printf("%f\n",C[j]);
	}
//	exit(-1);
}
for (i=0; i < daysiy; i++) {	

    /*----------------------------------------------
     *  Compute daily sediment discharge, Qs.
     *  Save both ratio's in memory or if there is
     *  not enough memory, save it to temp file.
     *----------------------------------------------*/
    ratio = Qsumtot[i]/Qbartotal[ep];      
	if (setstartmeanQandQs < 4){    	
	    Qs[i] = Psi[i] * pow(ratio, C[yr-syear[ep]]);        
	}
	if (setstartmeanQandQs == 4){
		Qs[i] = Psi[i] * Qsbarnew[ep] * pow(ratio, C[yr-syear[ep]]);
	
    /*-------------------------
     *  Check for NaNs in Qs
     *-------------------------*/
	    if (isnan(Qs[i])) {
    	    fprintf( stderr,"\nHydroSedload ERROR: Qs = nan \n");
	        fprintf( stderr," yr = %d, day = %d \n", yr, i );
    	    fprintf( stderr,"   Qnival[i]    = %e \n", Qnival[i] );
        	fprintf( stderr,"   Qrain[i]     = %e \n", Qrain[i] );
	        fprintf( stderr,"   Qexceedgw[i] = %e \n", Qexceedgw[i] );
    	    fprintf( stderr,"   Qice[i]      = %e \n", Qice[i] );
        	fprintf( stderr,"   Qss[i]       = %e \n", Qss[i] );
	        fprintf( stderr,"   baseflow[ep] = %e \n", baseflowtot[ep] ); /*  variables below here should be deleted */
    	    fprintf( stderr,"   ratio        = %e \n", ratio );
        	fprintf( stderr,"	Qsum[i]		 = %e \n", Qsumtot[i] );
	        fprintf( stderr,"	Qbar[ep]	 = %e \n", Qbartotal[ep] );
    	    fprintf( stderr,"   Psi[i]       = %e \n", Psi[i] );
        	fprintf( stderr,"   Qsbarnew[ep] = %e \n", Qsbarnew[ep] );
	        fprintf( stderr,"   C[i]         = %e \n", C[yr-syear[ep]] );
    	    fprintf( stderr,"   ep           = %d \n", ep );
        	fprintf( stderr,"   cbar         = %e \n", cbar );
	        err = 1;
    	}
	} //( end setstartmeanQandQs == 4 )

    /*---------------------------------
     *  Compute Qsannual and daily Cs
     *---------------------------------*/
    if (Qs[i] > 0.0) {
        Cs[i] = Qs[i] / Qsumtot[i];
        Qsannual += Qs[i] * dTOs;       
    } else
    	Cs[i] = 0.0;
    Csannual += Cs[i]*dTOs;

    /*----------------------------
     *  Compute threshold (m3/s)
     *----------------------------*/
//     threshbed[ep]=(Qbar[ep]/3);

    /*--------------------------------------------------------
     *  Compute bedload (kg/s), if above threshold flow rate
     *--------------------------------------------------------*/
//     if (Qsum[i] > threshbed[ep]) {
           Qb[i] = trnfac * rslope[ep] * pow( Qsumtot[i], alphabed );
           Qbedannual += Qb[i] * dTOs;
//     }

    /*-------------------------------
     *  Compute sediment per outlet
     *-------------------------------*/
	if (outletmodelflag == 1 && (setstartmeanQandQs > 2)){
	
	/*----------------------------------------------------------------
	 *  If there is more than 1 event occuring during a single year,
	 *  keep tracking on what they it is, to change the Qbar on that
	 *  day.
	 *----------------------------------------------------------------*/
		if ( numberday[y] == i && eventsperyear > 0){
			y++;
			eventsperyear--;
		}
		
    /*-----------------------------------
     *  Generate the random number, Psi
     *  Generate the random number, C
     *  (Can be normal or uniform.)
     *-----------------------------------*/	
		if (i == 0)
		for (j=0; j < daysiy; j++) {
			for (p=0; p<maxnoutlet; p++){
				if (outletpct[p][ep][eventcounter-eventsperyear] != 0.0){
					test = (Qsbartotoutlet[ep][p]/Qsoutletdummy)*Qsbartot[ep];
					if (test == 0.000 ){
						printf("test=%f",test);
						exit(-1);
					}
					cbaroutlet[p]	= (1.4 - (0.025 * Tbar) + (0.00013 * H) + (0.145 *log (test)));
					soutlet[p]		= (0.17 + (0.0000183 * Qbar[ep][p][eventcounter-eventsperyear]));
					unit_normal2 = ranarray[i + daysiy];
					normal2 = (soutlet[p] * unit_normal2) + cbaroutlet[p];
					Coutlet[j][p] = normal2;
					Coutletannual[p] +=Coutlet[j][p];
				}
			}
		}	
		for (p=0; p<maxnoutlet; p++){
			if (outletpct[p][ep][eventcounter-eventsperyear] != 0.0){	
    /*-------------------------
     *  Compute Qs per outlet
     *-------------------------*/			
				ratio = Qsum[i][p]/Qbar[ep][p][eventcounter-eventsperyear];
				if (setstartmeanQandQs == 3){
					Qsoutlet[i][p] = Psi[i] * pow((Qsbartotoutlet[ep][p]/Qsoutletdummy), Coutlet[yr-syear[ep]][p]) * pow(ratio, Coutlet[yr-syear[ep]][p]);
				}
				if (setstartmeanQandQs == 4){
					Qsoutlet[i][p] = Psi[i] * pow((Qsbartotoutlet[ep][p]/Qsoutletdummy), Coutlet[yr-syear[ep]][p]) * Qsbarnew2[ep] * pow(ratio, Coutlet[yr-syear[ep]][p]);
				
    /*-------------------------
     *  Check for NaNs in Qs
     *-------------------------*/
					if (isnan(Qsoutlet[i][p])) {
    					fprintf( stderr,"\nHydroSedload ERROR: Qsoutlet[day][nr] = nan \n");
						fprintf( stderr," yr = %d, day = %d, outlet = %d \n", yr, i, p );
						fprintf( stderr,"   Qnival[i]    = %e \n", Qnival[i] );
						fprintf( stderr,"   Qrain[i]     = %e \n", Qrain[i] );
						fprintf( stderr,"   Qexceedgw[i] = %e \n", Qexceedgw[i] );
						fprintf( stderr,"   Qice[i]      = %e \n", Qice[i] );
						fprintf( stderr,"   Qss[i]       = %e \n", Qss[i] );
						fprintf( stderr,"   baseflow[ep] = %e \n", baseflowtot[ep] ); /*  variables below here should be deleted */
						fprintf( stderr,"   ratio        = %e \n", ratio );
						fprintf( stderr,"	Qsumoutlet	 = %e \n", Qsum[i][p] );
						fprintf( stderr,"	Qbar[ep]	 = %e \n", Qbartotal[ep] );
						fprintf( stderr,"   Psi[i]       = %e \n", Psi[i] );
						fprintf( stderr,"   Qsbarnew2[ep]= %e \n", Qsbarnew2[ep] );
						fprintf( stderr,"   Coutlet[i][p]= %e \n", Coutlet[i][p] );
						fprintf( stderr,"   cbaroutlet[p]= %e \n", cbaroutlet[p] );
						fprintf( stderr,"	soutlet[p]   = %e \n", soutlet[p] );
						err = 1;
					}	
				}
				Qsannualoutlet[p] += Qsoutlet[i][p] * dTOs;
			
    /*--------------------------------------------
     *  Compute Qsannual and daily Cs per outlet
     *--------------------------------------------*/
				if (Qsoutlet[i][p] > 0.0)
					Csoutlet[i][p] = Qsoutlet[i][p] / Qsum[i][p];
				else Csoutlet[i][p] = 0.0;			
				Csannualoutlet[p] += Csoutlet[i][p] * dTOs;
    
    /*-------------------------------------
     *  Compute bedload (kg/s) per outlet
     *-------------------------------------*/			
				Qboutlet[i][p] = trnfac * rslope[ep] * pow( Qsum[i][p], alphabed );
				Qbedannualoutlet[p] += Qboutlet[i][p] * dTOs;
			}
			else Qsoutlet[i][p] = 0.0;
		} /* end for outlets*/
	}	
 }  /* end for loop over days */
 

Qsgrandtotal[ep] += Qsannual;
Csgrandtotal[ep] += Csannual;
if ( outletmodelflag == 1 && setstartmeanQandQs > 2 )
	for (p=0; p<maxnoutlet; p++){ 
		Qsgrandtotaloutlet[ep][p] += Qsannualoutlet[p];
		Csgrandtotaloutlet[ep][p] += Csannualoutlet[p];
		Coutlettotal[ep][p] +=Coutletannual[p];					
	}

/*---------------
 *  Free memory
 *---------------*/	
	freematrix1D( (void*)sigmaoutlet);
	freematrix1D( (void*)soutlet);
	freematrix1D( (void*)cbaroutlet);
	freematrix1D( (void*)Coutletannual);
	freematrix2D( (void**)Coutlet, daysiy);
	freematrix2D( (void**)Qsbartotoutlet, maxepoch);
	
 return(err);
}  /* end of HydroSedLoad */

