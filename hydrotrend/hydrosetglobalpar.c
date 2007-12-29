/*
 *  HydroSetGlobalPar.c
 *
 *  Author:   A.J. Kettner (February 2003)
 *
 *
 * Variable		Def.Location	Type		Units	Usage
 * --------		------------	----		-----	-----
 * err			various			int			-		error flag, halts program
 *
 */

#include <stdlib.h>

#include "hydrofree_mem.h"
#include "hydroclimate.h"
#include "hydroinout.h"
#include "hydroparams.h"
#include "hydrotimeser.h"
#include "hydroalloc_mem.h"
#include "hydroreadclimate.h"

/*---------------------
 *  Start the program
 *---------------------*/
int hydrosetglobalpar()
{
	
int err, verbose, dumint, i;
char chs[120], dumchar;
double dumlon, dumlat, dumlapserate;
long counter;
err = 0;
dumlon = 0;
verbose = 0;

/*------------------------------------
 *  Start looking up lapserate value
 *------------------------------------*/
	if (lapserateflag == 1){	
		if (verbose) printf("Opening %s... \n",fnamelapserate);
		if ( (fidlapserate = fopen(fnamelapserate,"r")) == NULL) {
			fprintf(stderr, "  HydroSetGlobalPar ERROR: Unable to open the lapserate table file %s \n",fnamelapserate);
			exit(-1);
		}
		dumint = 5;
		ep=0;
		for (ep=0; ep<nepochs; ep++ ){
			if (lon > 357.5)
				lon = 0.0;			
			fgets( chs, 120, fidlapserate );
			fgets( chs, 120, fidlapserate );
			sscanf( chs, "%li\n", &counter );
			for (i=0; i<counter; i++)
				while ((dumlon < lon && dumint == 5) || (dumlat > lat && dumint == 5)) {
					fgets( chs, 120, fidlapserate );
					dumint = sscanf( chs, "%lf%c %lf%c %lf\n", &dumlon,&dumchar, &dumlat,&dumchar, &dumlapserate );
					if (dumlon >= lon)
						while (dumlat > lat && dumint == 5){
							fgets( chs, 120, fidlapserate );
							dumint = sscanf( chs, "%lf%c %lf%c %lf\n", &dumlon,&dumchar, &dumlat,&dumchar, &dumlapserate );
	    	    	        if (dumlat <= lat){
    	    	    	    	lapserate[ep] = (dumlapserate / 1000.0);
    	    	    	    	i = counter;
		    	            }
		                }	                
				}
			if (dumint != 5){
				fprintf(stderr, "  HydroSetGlobalPar ERROR: In lapserate table file %s \n",fnamelapserate);
				fprintf(stderr, "     File is corrupt, Unable to read all the variables\n" );		
				err++;
			}
			rewind(fidlapserate);
		}
		fclose(fidlapserate);				
	}

/*-----------------------------------
 *  Start setting velocitypow value
 *-----------------------------------*/ 
	if (velpowflag == 1)
		for (ep=0; ep<nepochs; ep++ )
		 if (velpow[ep] == -9999)
			velpow[ep] = 0.1;
	
/*------------------------------
 *  Start setting widpow value
 *------------------------------*/ 	
	if (widpowflag == 1)
		for (ep=0; ep<nepochs; ep++ )
			if (widpow[ep] == -9999)
				widpow[ep] = 0.5;

	for (ep=0; ep<nepochs; ep++ )
		if ( widpowflag == 1 || velpowflag == 1){
   			depcof[ep] = 1/(velcof[ep]*widcof[ep]);
			deppow[ep] = 1 - velpow[ep] - widpow[ep];
		}
			
	return (err);
} /* end HydroSetGlobalPar.c */

