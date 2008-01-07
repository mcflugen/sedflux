//---
//
// This file is part of sedflux.
//
// sedflux is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// sedflux is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with sedflux; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---

/*
 * PlumeOut1.c	Prints the data to the output files for Plume (standalone)
 *		using MATLAB Binary file format
 *
 *	A one multidimensional file is written for each of the major variables:
 *	(where fname = the user input file name)
 *
 * filename	internal variable	description
 * --------	-----------------	-----------
 * fname.deps	deps(lx,ly,ngrains)	deposit thickness for each grain size
 * fname.ncnc	ncnc(lx,ly,ngrains)	non-conservative surface concentrations
 * fname.ccnc	ccnc(lx,ly,ngrains)	conservative values of surface concentrations
 * fname.sln	sln(lx,ly)		salinity (or any conservative tracer)
 * fname.ualb	ualb(lx,ly)		magnitude of the albertson velocities
 *
 *	Each file contains a header followed by the xval and yval arrays
 *	followed by the data arrays.
 *
 *	The header contains (one line for each variable):
 *		lx
 *		ly
 *		ngrains
 *		river.d0
 *
 *	Followed by an arrays of:
 *		xval(1:lx)
 *		yval(1:ly)
 *
 *	Then data arrays of the form:
 *		ccnc(1,1,1)  ccnc(1,2,1)  ... ccnc(1,ly,1)
 *		ccnc(2,1,1)  ccnc(2,2,1)  ... ccnc(2,ly,1)
 *		...
 *		ccnc(lx,1,1) ccnc(lx,2,1) ... ccnc(lx,ly,1)
 *
 *		ccnc(1,1,2) ...
 *		...		ccnc(lx,ly,2)
 *
 *		ccnc(1,1,ngrain) ...
 *		...		ccnc(lx,ly,ngrain)
 *
 *	Author:		M.D. Morehead
 *	Original:	May 1998
 *
 */
#include "plumeinput.h"
#include "plumevars.h"
#include <stdio.h>
#include <string.h>

int plumeout1(char fname[] , Plume_enviro *env , Plume_grid *grid )
{
   FILE	*fiddata;
   char	fndata[20];
   int	ii, jj, kk, nn, zz;

   zz = 0;

   // loop through once for each parameter
   for( kk=0; kk<5; kk++ )
   {
      /*
       *	Create the file name
       */
      strcpy( fndata, fname );
      if( kk == 0 )
         strcat( fndata, ".ccnc" );
      else if( kk == 1 )
         strcat( fndata, ".ncnc" );
      else if( kk == 2 )
         strcat( fndata, ".deps" );
      else if( kk == 3 )
         strcat( fndata, ".sln" );
      else
         strcat( fndata, ".ualb" );
      /*
       *	Open the output file
       */
      if( (fiddata = fopen(fndata,"w")) == NULL)
      {
         fprintf( stderr ,
                  "  PlumeOut1 ERROR: Unable to open the output file. \n");
         fprintf(stderr, "    fndata = %s \n", fndata);
         eh_exit(1);
      }
      /*
       *	Write the header, xval and yval arrays
       */
      fwrite(       &grid->lx, sizeof(int),   1, fiddata );
      fwrite(       &grid->ly, sizeof(int),   1, fiddata );
      if( kk == 0 || kk == 1 || kk == 2 )
         fwrite(  &env->n_grains, sizeof(int),   1, fiddata );
      else
         fwrite( &zz             , sizeof(int)   , 1 , fiddata );
      fwrite( &env->river->d0 , sizeof(double) , 1 , fiddata );
      /*
       *	Write out the xval and yval arrays
       */
      for( ii=0; ii<grid->lx; ii++ )
         fwrite( &grid->xval[ii], sizeof(double), 1, fiddata );
      for( jj=0; jj<grid->ly; jj++ )
         fwrite( &grid->yval[jj], sizeof(double), 1, fiddata );
      /*
       *	Write the data array
       */
      if( kk == 0  ) {
         for( nn=0; nn<env->n_grains; nn++ ) {
   	 for( ii=0; ii<grid->lx; ii++ ) {
   	    for( jj=0; jj<grid->ly; jj++ ) {
   	       fwrite( &grid->ccnc[ii][jj][nn], sizeof(double), 1, fiddata );
      }  }  }  }
      else if( kk == 1 ) {
         for( nn=0; nn<env->n_grains; nn++ ) {
   	 for( ii=0; ii<grid->lx; ii++ ) {
   	    for( jj=0; jj<grid->ly; jj++ ) {
   	       fwrite( &grid->ncnc[ii][jj][nn], sizeof(double), 1, fiddata );
      }  }  }  }
      else if( kk == 2 ) {
         for( nn=0; nn<env->n_grains; nn++ ) {
   	 for( ii=0; ii<grid->lx; ii++ ) {
   	    for( jj=0; jj<grid->ly; jj++ ) {
   	       fwrite( &grid->deps[ii][jj][nn], sizeof(double), 1, fiddata );
      }  }  }  }
      else if( kk == 3 ) {
         for( ii=0; ii<grid->lx; ii++ ) {
   	 for( jj=0; jj<grid->ly; jj++ ) {
   	    fwrite( &grid->sln[ii][jj], sizeof(double), 1, fiddata );
      }  }  }
      else {
         for( ii=0; ii<grid->lx; ii++ ) {
   	 for( jj=0; jj<grid->ly; jj++ ) {
   	    fwrite( &grid->ualb[ii][jj], sizeof(double), 1, fiddata );
      }  }  }
        /* */ 
      fclose(fiddata);
   } /* end for(kk) */
   return(0);
} // end of PlumeOut1
   
