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
 * PlumeMass	Calculate mass balance checks for PLUME
 *
 *	Author:		M.D. Morehead
 *	Original:	May 1998
 *
 */

#include <glib.h>
#include "utils.h"
#include "plumeinput.h"
#include "plumevars.h"

/**
@memo Calculate the mass of sediment entering the plume.

@doc Calculate the mass of sediment that enters the plume and compare it
to the amount of sediment that the plume deposits.  If the two masses do
not agree, then scale the deposit by an amount to make them agree.  The 
multi-grain size plume is treated as a superposition of single grain size
plume.  We balance the mass for each of these plumes individually.

*/
int plumemass( Plume_enviro *env , Plume_grid *grid , Plume_mass_bal *mb )
{
#ifdef DBG
   int xi[4];
   Plume_ocean ocean = *(env->ocean);
#endif
   int ii, jj, nn, err;
   double *mass_in, *mass_out;
   Plume_river river = *(env->river);
   Plume_sediment *sedload = env->sed;

   mass_in  = eh_new0( double , env->n_grains );
   mass_out = eh_new0( double , env->n_grains );

   err = 0;
   mb->Qsr = 0.0;
   mb->Tsr = 0.0; 
   mb->Qsw[0] = 0.0; mb->Qsw[1] = 0.0; mb->Qsw[2] = 0.0; mb->Qsw[3] = 0.0;
   mb->Tsd[0] = 0.0; mb->Tsd[1] = 0.0;

/*
   // Flux of sediment out of the river
   // Qs(kg/m^3 m^3/s = kg/s)
   // Ts = Qs*dt (kg/s s/day = kg/day)
   for( nn=0 ; nn<ngrains ; nn++ )
      Qsr = Qsr + river.Cs[nn]*river.Q;
   Tsr = Qsr*dTOs;
*/

   // calculate the mass of each grain size that enters the plume on a 
   // daily time step.
   for ( nn=0 ; nn<env->n_grains ; nn++ )
      mass_in[nn] = (river.Cs[nn]<.001)?0:(river.Cs[nn]*river.Q*dTOs);

#ifdef DBG
   // 1) Flux across any cross section perpendicular to the jet axis
   // must be constant and equal to flux out of river for
   //    a) the conservative plume
   //    b) in fjords or ocean.vo = 0
   // test 4 cross-sections close to 5, 10, 20, and 40 km
   if( opt->fjrd || opt->strt )
   {
      xi[0] = (int)rnd( 5000/grid->dx);
      xi[1] = (int)rnd(10000/grid->dx);
      xi[2] = (int)rnd(20000/grid->dx);
      xi[3] = (int)rnd(40000/grid->dx);

      for( ii=0 ; ii<4 ; ii++ )
      {
         if ( grid->lx > xi[ii] )
         {
   	    for ( jj=0 ; jj<grid->ly ; jj++ )
            {
   	       for ( nn=0 ; nn<env->n_grains ; nn++ )
               {
                  mb->Qsw[ii] = mb->Qsw[ii] 
            	              +   grid->ccnc[xi[ii]][jj][nn]
                                * grid->ualb[xi[ii]][jj]
                                * grid->dy
                                * river.d0;
   	       }
   	    }
         }
      }
   }
#endif

/*
   // 2) Total deposit should equal total sediment out of the river
   //    (m/day kg/m^3 m^2 = kg/day)
   for( ii=0; ii<lx; ii++ ) {
      for( jj=0; jj<ly; jj++ ) {
         for( nn=0; nn<ngrains; nn++ ) {
            Tsd[0] = Tsd[0] + deps[ii][jj][nn]*sedload[nn].rho*dx*dy;
         }
      }
   }
*/

   // calculate the mass of each grain size that is deposited by the
   // the plume.
   for ( ii=0 ; ii<grid->lx ; ii++ )
      for ( jj=0 ; jj<grid->ly ; jj++ )
         for ( nn=0 ; nn<env->n_grains ; nn++ )
            mass_out[nn] += grid->deps[ii][jj][nn]
                          * sedload[nn].rho
                          * grid->dx
                          * grid->dy;

#ifdef MASS_CHECK
   // Indicate if deposit != discharged sediment
   merr = (mb->Tsd[0] - mb->Tsr)/mb->Tsr;
   if( fabs(mb->merr) > mberr ) {
      fprintf(stderr,"----------------------------\n");
      fprintf(stderr,"Mass Balance ERROR in PLUME \n");
      fprintf(stderr,"----------------------------\n");
      fprintf(stderr,"   Mass of deposit != Mass discharged by river \n");
      fprintf(stderr,"   abs( ( Tsd[0] - Tsr )/ Tsr ) > mberr \n");
      fprintf(stderr,"   Mass of deposit (Tsd[0])	= %g (kg) \n", mb->Tsd[0]);
      fprintf(stderr,"   Mass from River (Tsr)	= %g (kg) \n", mb->Tsr);
      fprintf(stderr,"   ( Tsd[0] - Tsr )/Tsr)	= %g \n", mb->merr);
      fprintf(stderr,"   Max allowed error (mberr)	= %g \n", mberr);
      fprintf(stderr,"   mberr defined in PlumeVars.h \n\n");
      fprintf(stderr,"   Plume continuing, output will be questionable. \n\n");
      fprintf(stderr,"----------------------------\n\n");

      if ( opt->o1 )
      {
         fprintf(fidlog,"----------------------------\n");
         fprintf(fidlog,"Mass Balance ERROR in PLUME \n");
         fprintf(fidlog,"----------------------------\n");
         fprintf(fidlog,"   Mass of deposit != Mass discharged by river \n");
         fprintf(fidlog,"   abs( ( Tsd[0] - Tsr )/ Tsr ) < mberr \n");
         fprintf(fidlog,"   Mass of deposit (Tsd[0])	= %g (kg) \n", mb->Tsd[0]);
         fprintf(fidlog,"   Mass from River (Tsr)	= %g (kg) \n", mb->Tsr);
         fprintf(fidlog,"   ( Tsd[0] - Tsr )/Tsr)	= %g \n", mb->merr);
         fprintf(fidlog,"   Max allowed error (mberr)	= %g \n", mberr);
         fprintf(fidlog,"   mberr defined in PlumeVars.h \n\n");
         fprintf(fidlog,"   Plume continuing, output will be questionable. \n\n");
         fprintf(fidlog,"----------------------------\n\n");
      }
      err = 1;
   } 

#endif // end mass_check

#ifdef DBG

   printf("   Mass of deposit (Tsd[0])	= %g (kg) \n", mb->Tsd[0]);
   printf("   Mass from River (Tsr)	= %g (kg) \n", mb->Tsr);
   printf("   river.Q	= %g (m^3/s) \n", river.Q );
   printf("   river.u0	= %g (m/s) \n", river.u0 );
   printf("   river.b0	= %g (m) \n", river.b0 );
   printf("   river.d0	= %g (m) \n", river.d0 );
   printf("   river.Cs[0]	= %g (kg/m^3) \n", river.Cs[0] );
   printf("   river.Cs[1]	= %g (kg/m^3) \n", river.Cs[1] );
   printf("   river.Cs[2]	= %g (kg/m^3) \n", river.Cs[2] );
   printf("   river.Cs[3]	= %g (kg/m^3) \n", river.Cs[3] );
   printf("   sedload.lambda[0]	= %g (1/s) \n", sedload[0].lambda );
   printf("   sedload.lambda[1]	= %g (1/s) \n", sedload[1].lambda );
   printf("   sedload.lambda[2]	= %g (1/s) \n", sedload[2].lambda );
   printf("   sedload.lambda[3]	= %g (1/s) \n", sedload[3].lambda );
   printf("   ngrains	= %d  \n", env->n_grains );
   printf("   ocean.Cw	= %g (kg/m^3) \n", ocean.Cw );
   printf("   ocean.vo	= %g (m/s) \n", ocean.vo );
   printf("   ocean.vdirection	= %g (degN) \n", ocean.vdirection );
   printf("   river.rdirection	= %g (degN) \n", river.rdirection );
   printf("   ocean.cc	= %g (%%) \n", ocean.cc );
   printf("   river.rma	= %g (degN) \n", river.rma );
   printf("   kwf	= %d  \n", opt->kwf );
   printf("   lat	= %g (degN) \n", env->lat );

#endif

/*
   // Corrected deposit and total mass Tsd[1]
   for( ii=0; ii<lx; ii++ ) {
      for( jj=0; jj<ly; jj++ ) {
         for( nn=0; nn<ngrains; nn++ ) {
            deps[ii][jj][nn] = deps[ii][jj][nn]*(Tsr/Tsd[0]);
            Tsd[1] = Tsd[1] + deps[ii][jj][nn]*sedload[nn].rho*dx*dy;
          }
      }
   }
*/

   // scale the final deposit to balance the mass.
   for( ii=0 ; ii<grid->lx ; ii++ )
      for( jj=0 ; jj<grid->ly ; jj++ )
         for( nn=0 ; nn<env->n_grains ; nn++ )
            if ( mass_in[nn]>0 )
               grid->deps[ii][jj][nn] *= mass_in[nn]/mass_out[nn];

   for ( nn=0 ; nn<env->n_grains ; nn++ )
      mass_out[nn] = 0;

   eh_free( mass_in  );
   eh_free( mass_out );

   return(err);

} // end of PlumeMass

