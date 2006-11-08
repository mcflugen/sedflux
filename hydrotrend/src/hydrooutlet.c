/*
 *  HydroOutlet.c
 *
 *
 *  Author:    A.J. Kettner (April 2003)
 * 
 *  Contains 5 functions / subroutines for multiple outlet option
 * 	in HYDORTREND.
 * 
 *  1) HydroOutletFraction
 *  2) HydroSetNumberOutlet
 *  3) HydroQFractionShuffle
 *  4) HydroAllocMemOutlet
 *  4a)Hydroallocmemoutlet1
 *  5) HydroFreeMemOutlet
 * 
 */

#include <stdlib.h>
#include <math.h>
#include "hydroclimate.h"
#include "hydroinout.h"
#include "hydroparams.h"
#include "hydrotimeser.h"
#include "hydroreadclimate.h"
#include "hydroalloc_mem.h"
#include "hydrofree_mem.h"
#include "hydrornseeds.h"


/*  1) HydroOutletFraction
 *
 * If the discharge fraction per outlet is not specified by the user
 * and number of outlets is >1, hydrotrend computes the fraction based
 * on the number of outlets.
 * (If number of outlets changes during the ep, hydrotrend calculates
 * the fraction per event).
 * The calculation of the discharge fraction is based on x=y^2. y 
 * "represents" the number of outlets, and are values between 0 and 1.
 * (y = 1/(nr of outlets + 1)). So y0 = y*1; y1 = y*2; y2 = y*3 etc.
 * x is the power of y. As the total off all x must be equal to 1 (100% 
 * of the discharge is leaving the outlets) each x has to be devide by
 * the sum of all x.
 * 
 * 
 * Variable		Def.Location		Type		Units	Usage
 * --------		------------		----		-----	-----
 * check		HydroOutletFraction	double		-		Check if total fraction is 1.0
 * err			various				int			-		error flag, halts program
 * fdistance	HydroOutletFraction	double		-		
 * p			HydroOutletFraction	int			-		counter
 * Qtempfractot	HydroOutletFraction	double		-
 * totalpct		HydroOutletFraction	double		-
 * *tempfrac	HydroOutletFraction	double		-
 * *Qpowtempf	HydroOutletFraction	double		-
 * 
 */
 
/*--------------------------------
 *  Start of HydroOutletFraction
 *--------------------------------*/
int hydrooutletfraction(int x)
{

/*-------------------
 *  Local Variables
 *-------------------*/
int err,p;
double *tempfrac, *Qpowtempf;
double fdistance, Qtempfractot, totalpct, check;

/*------------------------
 *  Initialize variables
 *------------------------*/
err = 0;
Qtempfractot = 0.0;
totalpct = 0.0;

/*---------------------------------------
 *  Allocate memory for multiple outlet
 *---------------------------------------*/
tempfrac	= malloc1d( maxnoutlet, double );
Qpowtempf	= malloc1d( maxnoutlet, double );

/*----------------------------------
 *  Set outlet fraction per outlet
 *----------------------------------*/ 
	for (p=0; p<maxnoutlet; p++)
	tempfrac[p] = 0.0;
	fdistance = 1.0 / (noutlet+1);
	for (p=0; p<noutlet; p++){
		tempfrac[p] = fdistance * (1 + p);
		Qpowtempf[p] = pow(tempfrac[p],2);
		Qtempfractot += Qpowtempf[p];
	}	
	for (p=0; p<noutlet; p++){
		outletpct[p][ep][x] = Qpowtempf[p]/Qtempfractot;
		totalpct += outletpct[p][ep][x];
	}
	check = (totalpct - 1.0);
	if (fabs(check) > masscheck )
		err++;

/*---------------
 *  Free memory
 *---------------*/
	freematrix1D( (void*)tempfrac );
	freematrix1D( (void*)Qpowtempf );

	return (err);
} /* end of hydrooutletfraction */



/*  2) hydroSetNumberOutlet
 *
 * If the number of outlets is not specified by the user hydrotrend
 * computes randomly the number of outlet between a given range.
 * It will do this per event. So with each event (flood) hydrotrend
 * calculates a new number of outlets.
 * 
 * 
 * Variable		Def.Location			Type		Units	Usage
 * --------		------------			----		-----	-----
 * dumint		HydroSetNumberOutlet	int			-
 * dumflt		HydroSetNumberOutlet	flt			-
 * err			various					int			-		error flag, halts program
 * noutletoptionHydroSetNumberOutlet	int			-
 * x			HydroSetNumberOutlet	int			-
 * 
 */

/*---------------------------------
 *  Start of HydroSetNumberOutlet
 *---------------------------------*/
int hydrosetnumberoutlet(int x)
{

/*-------------------
 *  Local Variables
 *-------------------*/
int dumint, noutletoption;
double dumdbl;
float hydroran4(long *idum);
static int err = 0;
dumdbl = 0.5;

/*------------------------
 *  Set number of outlet
 *------------------------*/
	if ( x == minnoutlet )
		rnseed4 = -INIT_RAN_NUM_SEED;
	dumint = minnoutlet-1;
	while ( dumint < minnoutlet || dumint > maxnoutlet){
		dumdbl = (double)hydroran4(&rnseed4);
		if( 0 > dumdbl || dumdbl > 1 ){
			err++;			
			fprintf( stderr,"A function in HydroRan2 failed in HydroSetNumberOutlet (HydroOutlet) \n");
			fprintf( stderr," \t dumflt = %f: \t setting value to 0.5, x = %d \n", dumdbl, x);
			dumdbl=0.5;
		}		
		dumdbl = dumdbl*10.0;		
		dumint = (int)(dumdbl);
		if ( err > 1 ){
			fprintf( stderr, " ERROR in HydroSetNumberOutlet (HydroOutlet).\n" );
			fprintf( stderr, "\t Randomnummer generator failed twice: HydroTrend Aborted \n\n" );
			fprintf( fidlog, " ERROR in HydroSetNumberOutlet (HydroOutlet).\n" );
			fprintf( fidlog, "\t Randomnummer generator failed twice: HydroTrend Aborted \n\n" );
			exit(1);
		}						
	}
	noutletoption = dumint;
	return (noutletoption);	
} /* end of hydrosetnumberoutlet */



/*
 * 3) Hydroqfractionshuffle
 * 
 * Randomly shuffles the outletfractions.
 *
 *
 * Variable		Def.Location			Type	Units	Usage
 * --------		------------			----	-----	-----
 * dumflt		Hydroqfractionshuffle	float	-		temporary float
 * dvals[31]	Hydroqfractionshuffle	int		-		shuffled array of daily index values
 * err			Hydroqfractionshuffle	int		-		error flag, halts program
 * ii			Hydroqfractionshuffle	int		-		temporary loop counter
 * mnth			Hydroqfractionshuffle	int		-		month of the year
 * yy			Hydroqfractionshuffle	int		-		temporary integer
 * pp			Hydroqfractionshuffle	int		-		temporary integer
 *
 */

/*------------------------------------
 *  Start of Hydroqfractionshuffle.c
 *------------------------------------*/
int hydroqfractionshuffle( int k )
{

float hydroran5(long * idumd);
double dumdbl;
double dummy_double, *dummyoutletpct;
int yy, ii, err, *nvals, pp, *dummyvals, a;

/*---------------------------------------
 *  Allocate memory for multiple outlet
 *---------------------------------------*/
dummyoutletpct	= malloc1d( maxnoutlet, double );
nvals			= malloc1d( maxnoutlet, int);
dummyvals		= malloc1d( maxnoutlet, int );

/*------------------------
 *  Initialize variables
 *------------------------*/
err   = 0;
dumdbl = 0.5;
for( ii=0; ii<maxnoutlet; ii++ ){
	dummyoutletpct[ii] = outletpct[ii][ep][k];
	nvals[ii] = ii+1;
}

/*-------------------------------------
 *  shuffle the numbers of the outlet
 *-------------------------------------*/
	if ( k == 0)
		rnseed5 = -INIT_RAN_NUM_SEED/20; 
	for( ii=0; ii<maxnoutlet; ii++ ) {
		dumdbl = (double)hydroran5(&rnseed5);				/* get a uniform random number [0:1] */
		if( 0 > dumdbl || dumdbl > 1 ){
			fprintf( stderr,"A function in HydroRan2 failed in HydroShuffle.c \n");
			fprintf( stderr," \t dumdbl = %f: \t setting value to 0.5, ii = %d \n", dumdbl, ii);
			dumdbl=0.5;
			err++;
		}
		if ( k > 0 ){
			dummy_double = dumdbl*(float)maxnoutlet;
			yy = (int)rnd(dummy_double);			/* scale to a random day */
			if( yy == 0 ) yy += 1;
			nvals[ii] = nvals[yy-1];
			dummyvals[ii] = nvals[ii];		
			if ( ii !=0 ){
				a=1;
				for (pp=ii-1; pp>-1; pp-- )
					while ( dummyvals[ii] == dummyvals[pp] ){
						dummyvals[ii] = a;
						a++;
						pp = (ii-1);
					}
				nvals[ii] = dummyvals[ii];
			}
		}
	}
	for( ii=0; ii<maxnoutlet; ii++ ){
		nvals[ii] = nvals[ii]-1;
		outletpct[ii][ep][k] = dummyoutletpct[nvals[ii]];
	}

/*---------------
 *  Free memory
 *---------------*/
	freematrix1D( (void*)dummyoutletpct );
	freematrix1D( (void*)nvals );
	freematrix1D( (void*)dummyvals );

	return(err);
} /* end of HydroqfractionShuffle */


/*  4) HydroAllocmemOutlet
 *
 * Allocates memory for the outlet variables. The real allocation takes
 * place in Hydroalloc_mem.c
 *
 */
 
/*--------------------------------
 *  Start of HydroAllocmemOutlet
 *--------------------------------*/
void hydroallocmemoutlet(int ep)
{
	outletpcttotevents		= malloc2d( maxnoutlet, maxepoch, double );
	Qbedannualoutlet		= malloc1d( maxnoutlet, double );
	Qpeakperoutlet			= malloc1d( maxnoutlet, double );
	Qpeakperoutletall		= malloc2d( maxepoch, maxnoutlet, double );
	Qtotaloutletannual		= malloc1d( maxnoutlet, double );
	Qsgrandtotaloutlet		= malloc2d( maxepoch, maxnoutlet, double );
	Csgrandtotaloutlet		= malloc2d( maxepoch, maxnoutlet, double );
	Qsannualoutlet			= malloc1d( maxnoutlet, double );
	Csannualoutlet			= malloc1d( maxnoutlet, double );
	Coutlettotal			= malloc2d( maxepoch, maxnoutlet, double );
	Csoutlet				= malloc2d( daysiy, maxnoutlet, double );
	Qboutlet				= malloc2d( daysiy, maxnoutlet, double );
	Qsoutlet				= malloc2d( daysiy, maxnoutlet, double );
	Qsum					= malloc2d( maxday, maxnoutlet, double );
	Qgrandtotalperepoch	= malloc2d( maxepoch, maxnoutlet, double );
	Qdummy					= malloc2d( maxepoch, maxnoutlet, double );
	Qgrandtotaltotoutlet 	= malloc1d( maxnoutlet, double );
	Qsgrandtotaltotoutlet 	= malloc1d( maxnoutlet, double );
	Qpeakfloodtemp			= malloc2d( nyears[ep], daysiy, double );
	return;
} /* end of HydroAllocmemOutlet1 */


/*---------------------------------------
 *  Start of HydroAllocmemOutlet1; TEST
 *---------------------------------------*/
void hydroallocmemoutlet1(int ep)
{
	numberday				= malloc1d( eventsnr[ep], long ); //max allocatie is 365
	nroutlets				= malloc1d( eventsnr[ep], int );
	outletpct				= malloc3d( maxnoutlet, maxepoch, eventsnr[ep], double);
	Qgrandtotaloutlet		= malloc3d( maxepoche, maxnoutlet, eventsnr[ep], double );
	Qpeakevents				= malloc1d( eventsnr[ep], double );
	Qtotaloutlet			= malloc2d( maxnoutlet, eventsnr[ep], double );
	Qbar					= malloc3d( maxepoche, maxnoutlet, eventsnr[ep], double );
	Qpeakallevents			= malloc2d( maxepoche, eventsnr[ep], double );
	daysievent				= malloc1d( eventsnr[ep], long ); // max = days*years per event
	return;
} /* end of HydroAllocmemOutlet1 */



/*  5) HydroFreeMemOutlet
 *
 * Free memory for the outlet variables. To free the memory, it will use
 * the various subprograms in Hydrofree_mem.c
 *
 */
 
/*--------------------------------
 *  Start of HydroFreeMemOutlet
 *--------------------------------*/
void hydrofreememoutlet(int j)
{
	freematrix2D( (void**)outletpcttotevents, maxnoutlet );
	freematrix1D( (void*)Qbedannualoutlet );
	freematrix1D( (void*)Qpeakperoutlet );
	freematrix2D( (void**)Qpeakperoutletall, maxepoch);
	freematrix1D( (void*)Qtotaloutletannual );
	freematrix2D( (void**)Qsgrandtotaloutlet, maxepoch );
	freematrix2D( (void**)Csgrandtotaloutlet, maxepoch );
	freematrix1D( (void*)Qsannualoutlet );
	freematrix1D( (void*)Csannualoutlet );	
	freematrix2D( (void**)Coutlettotal, maxepoch );
	freematrix2D( (void**)Csoutlet, daysiy );
	freematrix2D( (void**)Qboutlet, daysiy );
	freematrix2D( (void**)Qsoutlet, daysiy );	
	freematrix2D( (void**)Qsum, maxday );
	freematrix2D( (void**)Qgrandtotalperepoch, maxepoch );	
	freematrix2D( (void**)Qdummy, maxepoch );	
	freematrix1D( (void*)Qgrandtotaltotoutlet );
	freematrix1D( (void*)Qsgrandtotaltotoutlet );
	freematrix2D( (void**)Qpeakfloodtemp, j );
	return;
} /* end of hydroFreeMemOutlet */

/*-------------------------------------
 *  Start of HydroFreeMemOutlet1 TEST
 *-------------------------------------*/
void hydrofreememoutlet1(int ep)
{
	freematrix1D( (void*)numberday );
	freematrix1D( (void*)nroutlets );
	freematrix3D( (void***)outletpct, maxnoutlet, maxepoch );
	freematrix3D( (void***)Qgrandtotaloutlet, maxepoche, maxnoutlet );
	freematrix1D( (void*)Qpeakevents );
	freematrix2D( (void**)Qtotaloutlet, maxnoutlet );
	freematrix3D( (void***)Qbar, maxepoche, maxnoutlet );
	freematrix2D( (void**)Qpeakallevents, maxepoche );
	freematrix1D( (void*)daysievent );				
	return;	
} /* end of hydroFreeMemOutlet1 */


/* end of HydroOutlet.c */
