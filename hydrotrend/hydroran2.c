/*
 *  HydroRan2.c
 *  This program contance a couple of functions, hydroran2 till hydroran5.
 *  The functions are the same but made to generate exactly the
 *  same numbers for every epoch when it goes 4 times through the
 *  loop in hydrotrend.c. (Once for calculating Qbar, second time
 *  to calculate Qs e.d.
 *
 *  Generates uniformly distributed numbers between 0.0 and 1.0.
 *  ran2.c From: "Numerical Recipes in C", p282, 2nd ed.
 *
 *  For the first deviate, use make sure the seed is negative;
 *  this initializes ran2 appropriately.
 *  For subsequent years, use the generated seed; idum should not
 *  be altered between successive deviates in a sequence.
 *
 */

#include <math.h>
#include "hydroparams.h"

#define	IM1	2147483563
#define	IM2	2147483399
#define	AM	(1.0/IM1)
#define	IMM1	(IM1-1)
#define	IA1	40014
#define	IA2	40692
#define	IQ1	53668
#define	IQ2	52774
#define	IR1	12211
#define	IR2	3791
#define	NTAB	32
#define	NDIV	(1+IMM1/NTAB)
#define	EPS	1.2e-7
#define	RNMX	(1.0-EPS)

#define	BIM1	2147483563
#define	BIM2	2147483399
#define	BAM	(1.0/BIM1)
#define	BIMM1	(BIM1-1)
#define	BIA1	40014
#define	BIA2	40692
#define	BIQ1	53668
#define	BIQ2	52774
#define	BIR1	12211
#define	BIR2	3791
#define	BNTAB	32
#define	BNDIV	(1+BIMM1/BNTAB)
#define	BEPS	1.2e-7
#define	BRNMX	(1.0-BEPS)

#define	CIM1	2147483563
#define	CIM2	2147483399
#define	CAM	(1.0/CIM1)
#define	CIMM1	(CIM1-1)
#define	CIA1	40014
#define	CIA2	40692
#define	CIQ1	53668
#define	CIQ2	52774
#define	CIR1	12211
#define	CIR2	3791
#define	CNTAB	32
#define	CNDIV	(1+CIMM1/CNTAB)
#define	CEPS	1.2e-7
#define	CRNMX	(1.0-CEPS)

#define	DIM1	2147483563
#define	DIM2	2147483399
#define	DAM	(1.0/DIM1)
#define	DIMM1	(DIM1-1)
#define	DIA1	40014
#define	DIA2	40692
#define	DIQ1	53668
#define	DIQ2	52774
#define	DIR1	12211
#define	DIR2	3791
#define	DNTAB	32
#define	DNDIV	(1+DIMM1/DNTAB)
#define	DEPS	1.2e-7
#define	DRNMX	(1.0-DEPS)

#include <stdio.h>

/*------------------------
 *  Start of HydroRan2.c
 *------------------------*/
float hydroran2(long *idum)
{
int jj;
long kk;
static long idum2=123456789;
static long iy=0;
static long iv[NTAB];
float temp;

/*----------------------------
 *  Initialize the generator
 *----------------------------*/
if (*idum <= 0) {
    if( -(*idum) < 1 ) *idum = 1;	  /* prevent idum = 0 */
    else *idum = -(*idum);
    idum2=(*idum);
    for (jj=NTAB+7; jj>=0; jj--) {	  /* load the shuffle table */
         kk=(*idum)/IQ1;
         *idum=IA1*(*idum-kk*IQ1)-kk*IR1;
         if( *idum < 0 ) *idum += IM1;
         if( jj < NTAB ){
         iv[jj] = *idum;
         }
    }
    iy = iv[0];
}

/*-----------------------
 *  Start the generator
 *-----------------------*/
kk = (*idum)/IQ1;
*idum = IA1*(*idum-kk*IQ1)-kk*IR1;
if (*idum < 0) *idum += IM1;
kk = idum2/IQ2;
idum2 = IA2*(idum2-kk*IQ2)-kk*IR2;
if (idum2 < 0) idum2 += IM2;
jj=iy/NDIV;
iy = iv[jj]-idum2;
iv[jj] = *idum;
if (iy < 1) iy += IMM1;
if ((temp=AM*iy) > RNMX) return RNMX;
else return temp;

}  /* end of HydroRan2.c */

/*------------------------
 *  Start of HydroRan3.c
 *------------------------*/
float hydroran3(long *idumb)
{
int jjj;
long kkk;
static long idum22=123456789;
static long iy2=0;
static long iv2[BNTAB];
float temp2;

/*----------------------------
 *  Initialize the generator
 *----------------------------*/
if (*idumb <= 0) {
    if( -(*idumb) < 1 ) *idumb = 1;	  /* prevent idum = 0 */
    else *idumb = -(*idumb);
    idum22=(*idumb);
    for (jjj=BNTAB+7; jjj>=0; jjj--) {	  /* load the shuffle table */
         kkk=(*idumb)/IQ1;
         *idumb=IA1*(*idumb-kkk*IQ1)-kkk*IR1;
         if( *idumb < 0 ) *idumb += BIM1;
         if( jjj < BNTAB ){
         iv2[jjj] = *idumb;
         }
    }
    iy2 = iv2[0];
}

/*-----------------------
 *  Start the generator
 *-----------------------*/
kkk = (*idumb)/BIQ1;
*idumb = BIA1*(*idumb-kkk*BIQ1)-kkk*BIR1;
if (*idumb < 0) *idumb += BIM1;
kkk = idum22/IQ2;
idum22 = BIA2*(idum22-kkk*BIQ2)-kkk*BIR2;
if (idum22 < 0) idum22 += BIM2;
jjj=iy2/BNDIV;
iy2 = iv2[jjj]-idum22;
iv2[jjj] = *idumb;
if (iy2 < 1) iy2 += BIMM1;
if ((temp2=BAM*iy2) > BRNMX) return BRNMX;
else return temp2;

}  /* end of HydroRan3.c */


/*------------------------
 *  Start of HydroRan4.c
 *------------------------*/
float hydroran4(long *idumc)
{
int j;
long k;
static long idum23=123456789;
static long iy3=0;
static long iv3[CNTAB];
float temp3;

/*----------------------------
 *  Initialize the generator
 *----------------------------*/
if (*idumc <= 0) {
    if( -(*idumc) < 1 ) *idumc = 1;	  /* prevent idum = 0 */
    else *idumc = -(*idumc);
    idum23=(*idumc);
    for (j=CNTAB+7; j>=0; j--) {	  /* load the shuffle table */
         k=(*idumc)/CIQ1;
         *idumc=CIA1*(*idumc-k*CIQ1)-k*CIR1;
         if( *idumc < 0 ) *idumc += CIM1;
         if( j < CNTAB ){
         iv3[j] = *idumc;
         }
    }
    iy3 = iv3[0];
}

/*-----------------------
 *  Start the generator
 *-----------------------*/
k = (*idumc)/CIQ1;
*idumc = CIA1*(*idumc-k*CIQ1)-k*CIR1;
if (*idumc < 0) *idumc += CIM1;
k = idum23/CIQ2;
idum23 = CIA2*(idum23-k*CIQ2)-k*CIR2;
if (idum23 < 0) idum23 += CIM2;
j=iy3/CNDIV;
iy3 = iv3[j]-idum23;
iv3[j] = *idumc;
if (iy3 < 1) iy3 += CIMM1;
if ((temp3=CAM*iy3) > CRNMX) return CRNMX;
else return temp3;

}  /* end of HydroRan4.c */


/*------------------------
 *  Start of HydroRan5.c
 *------------------------*/
float hydroran5(long *idumd)
{
int j;
long k;
static long idum5=123456789;
static long iy5=0;
static long iv5[DNTAB];
float temp5;

/*----------------------------
 *  Initialize the generator
 *----------------------------*/
if (*idumd <= 0) {
    if( -(*idumd) < 1 ) *idumd = 1;	  /* prevent idum = 0 */
    else *idumd = -(*idumd);
    idum5=(*idumd);
    for (j=DNTAB+7; j>=0; j--) {	  /* load the shuffle table */
         k=(*idumd)/DIQ1;
         *idumd=DIA1*(*idumd-k*DIQ1)-k*DIR1;
         if( *idumd < 0 ) *idumd += DIM1;
         if( j < DNTAB ){
	         iv5[j] = *idumd;
         }
    }
    iy5 = iv5[0];
}

/*-----------------------
 *  Start the generator
 *-----------------------*/
k = (*idumd)/DIQ1;
*idumd = DIA1*(*idumd-k*DIQ1)-k*DIR1;
if (*idumd < 0) *idumd += DIM1;
k = idum5/DIQ2;
idum5 = DIA2*(idum5-k*DIQ2)-k*DIR2;
if (idum5 < 0) idum5 += DIM2;
j=iy5/DNDIV;
iy5 = iv5[j]-idum5;
iv5[j] = *idumd;
if (iy5 < 1) iy5 += DIMM1;
if ((temp5=DAM*iy5) > DRNMX) return DRNMX;
else return temp5;

}  /* end of HydroRan5.c */

