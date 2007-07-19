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

/* description 
* main module of sakura
*/

/* parameters
* stopnumber: error detection
* headnode: node position of the flow head
* xhead: position of the flow head
* uhead: propagation velocity of the flow head
* CCMULTI & CCMULTInew: sediment concentration for each size fraction at nodes
* U, Utemp & Unew: flow velocity at nodes
* HH & HHnew: flow thickness at nodes
* CC & CCnew: sediment concentration at nodes
* SED: deposit mass at nodes
* SEDRATE: rate of deposition at nodes
* Init_U: flow velocity at the river mouth given in discharge files
* Init_C: sediment concentration at the river mouth given in discharge files 
* u, h, & c: velocity, thickness and concentration
*      subscript l, r, m means left, right and middle (or averaged)
*      subscript ll and rr mean 2 nodes next from the node being calculated
* sedrate, ustar & s: rate of deposition, shear velocity and slope at the node
* fluxatbed: sum of rate of deposition and erosion
* erosion: rate of erosion
* inflow_*: flow properties at river mouth
* outflow_*: flow properties at the downstream end
* xtravel: storage of head position
* smallh: length scale of the settling velocity
* porosity: calculated from density of deposit
* maxc: maximun value of sediment concentration for each loop
* totalsusp: amount of sediment in the flow
* av_graindensity, bulkdensitybottom: average value
*/

#include <stdio.h>
#include <math.h>
#include "utils.h"
#include "sakura_local.h"
#include "sakura.h"
//#include "sakura_utils.h"

/* supply duration in seconds */
#define SUPPLYTIME (60.0*60.0*12)

#undef YUSUKE

//double get_depth( int i , Eh_query_func get_depth , gpointer bathy_data );

//@Include: sakura.h

/**
The sakura turbidity current model.

@param Dx               Node spacing.
@param Dt               Time step.
@param Basin_Len        Length of the basin.
@param nNodes           The number of nodes in the model domain.
@param nGrains          The number of grain types in the flow.
@param Xx               The x-position of the model nodes.
@param Zz         
@param Wx         
@param Init_U           The initial velocity of the flow.
@param Init_C           The initial concentration of the flow.
@param Lambda           The removal rate of each grain type.
@param Stv
@param Rey
@param gDens
@param InitH            The initial height of the flow.
@param SupplyTime
@param DepositionStart
@param Fraction         The fraction of each grain type in the flow.
@param PheBottom        The fraction of each grain type in the bottom sediments.
@param DepositDensity   The bulk density of each grain type when it is deposited
                        on the sea floor.
@param OutTime
@param c                Physical constants for the flow.
@param Deposit          The deposition rates for each grain types at each node.
@param fp_data          The file pointer for output data.
*/
gboolean sakura ( double Dx         , double Dt              , double Basin_Len ,
             int nNodes        , int nGrains            , double Xx[]      ,
             double Zz[]       , double Wx[]            , double Init_U[]  ,
             double Init_C[]   , double *Lambda         , double *Stv      ,
             double *Rey       , double *gDens          , double InitH     ,
             double SupplyTime , double DepositionStart , double *Fraction ,
             double *bottom_f  , double *DepositDensity , double OutTime   ,
             Sakura_const_st* Const , double **Deposit       , FILE *fp_data )
{
   /*declare variables*/
   int  stopnumber = 0;
   int count, node, headnode, outIntv, i, n;
   double **CCMULTI, **CCMULTInew, **SEDMULTI; /* sediment concentration of each size fraction*/
   double *erosion, *outflow_cMULTI;           /* for multiple grainsize*/
   double time, xhead, uhead;
   double *U, *Utemp, *Unew;                   /* flow velocities at node position*/
   double *xtravel;                            /* save the position of the head*/
   double *SED, *SEDRATE, *DEPTH;              /* deposited sediment and its rate at node midpoint*/
   double *HH, *CC, *HHnew, *CCnew; /* flow thickness and sediment concentration at node midpoint*/
   double smallh, porosity;                    
   double u, ul, ur, ull, urr, cm, cl, cr, cll, crr, hm, hl, hr, hll, hrr, s, ustar; 
   double h, c, um, wl, wr, cnew, cnewi, sedrate, fluxatbed, dh; 
   double Ew, Ri, Ze; 
   double maxc, maxu, totalsusp;
   double outflow_c, outflow_h, outflow_u, outflow_cnew, outflow_hnew, outflow_unew;
   double inflow_u, inflow_h, inflow_c; 
   double av_graindensity, bulkdensitybottom;
   double uhead_1;
   double erode_depth, x;
   double depth_0, depth_1, depth_node;
   gboolean standalone=FALSE;

   double*          PheBottom      = eh_new( double , nGrains );
   Sakura_phe_st    phe_data;
   Sakura_cell_st   sediment;

   Sakura_phe_func get_phe_func   = Const->get_phe;
   Sakura_add_func add_func       = Const->add;
   Sakura_add_func remove_func    = Const->remove;
   Sakura_get_func get_depth_func = Const->get_depth;

   eh_require( bottom_f==NULL );

#if defined( SAKURA_STANDALONE )
   standalone = TRUE;
#endif

   // open ASCII output file
//   if ( standalone && (Outfp2 = fopen("sakura_out.txt", "w")) == NULL)
//   {
//      fprintf(stderr, "Err opening output file ...EXITING\n");
//      eh_exit(-1);
//   }

   // allocate memory
   // node values.
   // U     : flow velocity
   // Utemp : tentative velocity
   // Unew  : new velocity
   U       = eh_new0( double , nNodes );
   Utemp   = eh_new0( double , nNodes );
   Unew    = eh_new0( double , nNodes );

   // node minpoint values.
   // HH      : flow thickness
   // CC      : bulk concentration
   // SED     : deposited mass
   // SEDRATE : sedimentation rate
   // HHnew   : new flow thickness
   // CCnew   : new bulk concentration
   HH      = eh_new0( double , nNodes-1 );
   CC      = eh_new0( double , nNodes-1 );
   SED     = eh_new0( double , nNodes-1 );
   SEDRATE = eh_new0( double , nNodes-1 );
   DEPTH   = eh_new0( double , nNodes-1 );
   HHnew   = eh_new0( double , nNodes-1 );
   CCnew   = eh_new0( double , nNodes-1 );
   
   // position of the head at each time step
   xtravel = eh_new0( double , (int)(SupplyTime/Dt) );

   // matrix arrays for node values for each grain size.
   // CCMULTI    : concentration for multi grain size fractions
   // CCMULTInew : concentration for multi grain size fractions
   CCMULTI    = eh_new_2( double , nNodes , nGrains );
   CCMULTInew = eh_new_2( double , nNodes , nGrains );
   SEDMULTI   = eh_new_2( double , nNodes , nGrains );
   
   // rate of erosion
   erosion        = eh_new0( double , nGrains );
   outflow_cMULTI = eh_new0( double , nGrains );
   
// end of memory allocation.

   xhead = HMIN;
   uhead = Init_U[0];
   headnode = floor(xhead/Dx);

#if defined(YUSUKE)
// NOTE: removed by eric.  from here...
   for (i = 0, av_graindensity = 0, bulkdensitybottom = 0; i < nGrains; i++)
   {
      av_graindensity += gDens[i] * PheBottom[i];
      bulkdensitybottom += DepositDensity[i] * PheBottom[i];
   }
   porosity = 1.0 - bulkdensitybottom/av_graindensity;
// ... to here.
#endif

   eh_debug( "SupplyTime:%.2f, initU:%.2f, intC;%.2f, maxtime:%.2f" , SupplyTime , Init_U[0] , Init_C[0] , SupplyTime );

   for ( i=0 ; i<nNodes ; i++ )
      DEPTH[i] = Zz[i];

   /* START MAIN LOOP with Dt*/
   outIntv = (int)(OutTime / Dt);
   for ( time = 0.0, count =0 ;
         time <= SupplyTime && stopnumber ==0 ;
         time += Dt, count++ )
   {
      /* save node attributes to outfile*/
      if ( fp_data && !(count%outIntv)) 
      {
         fwrite( Xx  , nNodes-1 , sizeof(double) , fp_data );
         fwrite( U   , nNodes-1 , sizeof(double) , fp_data );
         fwrite( HH  , nNodes-1 , sizeof(double) , fp_data );
         fwrite( CC  , nNodes-1 , sizeof(double) , fp_data );
         fwrite( SED , nNodes-1 , sizeof(double) , fp_data );

//         outputData( Outfp2 , nNodes , time    , U     , HH    , CC ,
//                     SED    , Utemp  , SEDRATE , xhead , uhead , headnode );
      }
      
      // input from river mouth
      if (time <= SupplyTime)
      {
         if ( standalone )
            inflow_u = Init_U[count];
         else
            inflow_u = Init_U[0];
         U[0] = inflow_u; Utemp[0] = inflow_u;
         inflow_h = InitH; //HH[0] = inflow_h;
         //  inflow_c = (discharge_density[count] - Consts.rhoRW)/(av_graindensity - Consts.rhoRW);
         if ( standalone )
            inflow_c = Init_C[count];
         else
            inflow_c = Init_C[0];
      }
      else
      {
         inflow_u = 0; U[0] = inflow_u; Utemp[0] = inflow_u;
         inflow_h = HH[0];
         inflow_c = CC[0];
      }
            
      // free outflow at downstream end
      if (xhead <= Basin_Len)
      {
         outflow_u = 0;
        outflow_c = 0;
         outflow_h = 0;
         for (i = 0; i < nGrains; i++)
            outflow_cMULTI[i] = 0;
      }
      else if (xhead <= Basin_Len +Dx)
      {
         outflow_u = 0; 
         outflow_h = outflow_h + HH[nNodes-2] * U[nNodes-1] * Dt/Dx;
         outflow_c = CC[nNodes-2];
         for (i = 0; i < nGrains; i++)
            outflow_cMULTI[i] = CCMULTI[nNodes-2][i];
      }
      else
      {
         outflow_u = U[nNodes-1];
         outflow_h = HH[nNodes-2];
         outflow_c = CC[nNodes-2];
         for (i = 0; i < nGrains; i++)
            outflow_cMULTI[i] = CCMULTI[nNodes-2][i];
      }
   
      // STEP 1: calculate tentative velocity at t + 0.5Dt */
      // start from node =1 because velocity is given at upstream end (node=0)
      // calculate only within the flow (behind the head position) */
      headnode = eh_min(headnode, nNodes-1);
      smallh   = Stv[0] * Dt *2; 

      for (node = 1; node <= headnode; node++)
      {
         u     = U[node];
#if defined(YUSUKE)
         s     = - sin( atan( (DEPTH[node] - DEPTH[node-1])/Dx ));
#else
         // get the depths of the i-1 and i nodes from the sakura architecture
         // so that we can calculate the slope.
/*
         depth_0 = get_depth( node-1 , Consts.get_depth , Consts.depth_data );
         depth_1 = get_depth( node   , Consts.get_depth , Consts.depth_data );
*/

         depth_0 = get_depth_func( Const->depth_data , Xx[node-1] );
         depth_1 = get_depth_func( Const->depth_data , Xx[node]   );

         s       = - sin( atan( (depth_1-depth_0)/Dx ) );
#endif
         ustar = sqrt(Const->c_drag) * u;

         ul = U[node-1]; 
         cl = CC[node-1]; 
         hl = HH[node-1]; 

         if (node == nNodes-1)
         {
            cr = outflow_c;
            hr = outflow_h;
            ur = outflow_u;
            urr = outflow_u;
         }
         else if (node == nNodes-2)
         {
            cr = CC[node];
            hr = HH[node];
            ur = U[node+1];
            urr = outflow_u;
         }
         else
         {
            cr = CC[node];
            hr = HH[node];
            ur = U[node+1];
            urr = U[node+2];
         }
         
         if (node == 1)
            ull = inflow_u;
         else
            ull = U[node-2];

         // value at the node calculated from those at midpoints
         cm = 0.5 * ( cl + cr );
         hm = 0.5 * ( hl + hr );
         
         if (hm == 0)
            ;
         else if (hm < HMIN)
         {
            eh_warning( "hm too small in STEP 1 at count = %d, node = %d" , count , node );
            stopnumber = 1;
         }

         // compute water entrainment 
         Ew = 0.0;
         if (u != 0.0)
         {
            Ri = R * G * hm / eh_sqr(u);
            Ew = Const->e_a / (Const->e_b + Ri);
         }
                 
         // tentative varibales with dt = 0.5 Dt 
         Utemp[node] = u
                     + dudt( u  , ul , ur     , ull , urr       , hl    ,
                             hr , hm , cl     , cr  , cm        , ustar ,
                             s  , Ew , smallh , Dx  , Const->c_drag , Const->mu_water )
                       * Dt * 0.5;
         
      } // end of STEP1

      uhead_1 = uhead;   
      uhead = eh_max(Utemp[node-1], Utemp[node-2]);

      if (node <= 1)
         uhead = Utemp[0];
      else if (uhead > 0)
      {
         uhead = eh_min(uhead, 1.5 * pow(G * R * cl * hl * uhead, 0.3333));
      }
      else
      {
         uhead = eh_max(Utemp[node-1], Utemp[node-2]);
      }

      if ( uhead > 0 )
         ;
      else
      {
/*                     fprintf(stderr,"flow is going back! but cancelled at time=%f; uhead=%f; xhead=%f\n", time,uhead, xhead);
                     fprintf(stderr,"cl=%f; hl=%f; Utemp[node-1]=%f, Utemp[node-2]=%f\n", cl, hl, Utemp[node-1], Utemp[node-2]);
                     fprintf(stderr,"uhead_1=%f; uhead_2=%f\n", uhead_1, uhead_2);
                     fprintf(stderr,"LARGER=%f; pow=%f\n", LARGER(Utemp[node-1], Utemp[node-2]), SMALLER(uhead,  pow(cl*hl*uhead_2, 0.3333)));
                     fprintf(stderr,"pow=%f\n", pow(cl*hl*uhead_2, 0.3333));
                     stopnumber = 0;
*/
      }
            
      // START STEP2: calculate new flow thickness and sediment concentration
      // calculations at node midpoint for HH, CC and SED
      //  uses Utemp to get HHnew and CCnew
      maxc = 0.; totalsusp = 0.;

      for (node = 0; node <= headnode && node <=nNodes-2; node++)
      {
         h = HH[node];
         c = CC[node];
         
         ul = Utemp[node]; ur = Utemp[node+1];
         wl = Wx[node]; wr = Wx[node+1]; 
         if (node == nNodes-2)
            um = ul;
         else
            um = 0.5 * ( ul + ur );
         ustar = sqrt(Const->c_drag) * fabs(um); 

         if (node == 0)
         {
            hl = inflow_h; hr = HH[node+1];
            hll = inflow_h; hrr = HH[node+2];
         }
         else if (node == nNodes-2)
         {
            hl = HH[node-1]; hr = outflow_h;
            hll = HH[node-2]; hrr = outflow_h;
         }
         else if (node ==1)
         {
            hl = HH[node-1]; hr = HH[node+1];
            hll = inflow_h; hrr = HH[node+2];
         }
         else if (node == nNodes-3)
         {
            hl = HH[node-1]; hr = HH[node+1];
            hll = HH[node-2]; hrr = outflow_h;
         }
         else
         {
            hl = HH[node-1]; hr = HH[node+1];
            hll = HH[node-2]; hrr = HH[node+2];
         }
         
         //compute water entrainment 
         Ew = 0.0; Ri = 0;
         if (um != 0.0)
         {
            Ri = R * G * c * h / eh_sqr(um);
            Ew = Const->e_a / (Const->e_b + Ri);
         }
         
         // compute new HH
         HHnew[node] = h + Dt * dfdt(ul, ur, wl, wr, hl, hr, hll, hrr, h, Dx, Ew*fabs(um));

         if (HHnew[node] < 0)
         {
            eh_warning( "HHnew negative but cancelled at %d at %dth node" ,count,node);
            eh_warning( "old=%f, new=%f, dfdt=%f, left=%f, right=%f"      ,h,HHnew[node],dfdt(ul,ur,wl,wr,hl,hr,hll,hrr,h,Dx,Ew*um),tvdleft(ul,h,hl,hr,hll,hrr),tvdright(ur,h,hl,hr,hll,hrr));
            eh_warning( "ul:%f, ur:%f, hl:%f, h:%f, hr:%f", ul,ur,hl,h,hr);
            HHnew[node] = 0;
         }

//                     if (HHnew[node] > LARGER(InitH, fabs(DEPTH[node])) )
 //                        HHnew[node] = -DEPTH[node];

         // compute CCMULTI for each grain size fraction
         for (i = 0, Ze = 0, sedrate = 0, cnew = 0; i < nGrains; i++)
         {
            c = CCMULTI[node][i];
            smallh = Stv[i] * Dt * Ro;
            if (node == 0)
            {
               cl = inflow_c * Fraction[i]; cr = CCMULTI[node+1][i];
               cll = inflow_c * Fraction[i]; crr = CCMULTI[node+2][i];
            }
            else if (node == nNodes-2)
            {
               cl = CCMULTI[node-1][i]; cr = outflow_cMULTI[i];
               cll = CCMULTI[node-2][i]; crr = outflow_cMULTI[i];
            }
            else if (node ==1)
            {
               cl = CCMULTI[node-1][i]; cr = CCMULTI[node+1][i];
               cll = inflow_c* Fraction[i]; crr = CCMULTI[node+2][i];
            }
            else if (node == nNodes-3)
            {
               cl = CCMULTI[node-1][i]; cr = CCMULTI[node+1][i];
               cll = CCMULTI[node-2][i]; crr = outflow_cMULTI[i];
            }
            else
            {
               cl = CCMULTI[node-1][i]; cr = CCMULTI[node+1][i];
               cll = CCMULTI[node-2][i]; crr = CCMULTI[node+2][i];
            }
            
            if (HHnew[node] < HMIN)
            {
               cnewi = 0;
               eh_warning("HHnew too small in STEP2 at node = %d, Hnew = %f", node, HHnew[node]);
               //stopnumber = 1;
            }
            else
            {
               cnewi = (c * h + Dt * dfdt(ul, ur, wl, wr, hl*cl, hr*cr, hll*cll, hrr*crr, h*c, Dx, 0) )/HHnew[node];
               if (cnewi < -HMIN)
               {
                  eh_warning("negative new CC: t= %d, node= %d, i= %d", count, node, i);
                  eh_warning("cnew= %f, cold=%f", cnewi, c);
                  cnewi = 0;
                  stopnumber = 0;
               }
            } //cnewi is the new concentration due to sediment transport by the flow
            
         //compute sediment deposition and erosion from cnewi

/*                      if (Stv[i] != 0.0 && Rey[i] > 2.36)
                              Ze = pow(Rey[i], 0.6) * ustar / Stv[i];
                        else if (Stv[i] != 0.0)
                              Ze = 0.586 * pow(Rey[i], 1.23) * ustar / Stv[i];
                        erosion[i] = 1.3E-7 * pow(Ze, 5.0) / (1.0 + (1.3E-7 / 0.3) * pow(Ze, 5.0)); 
*/

#if !defined(YUSUKE)
//
// NOTE: added by eric from here...
//
            // here we get the PheBottom at the node location.
            erode_depth     = (Const->c_drag*(1+cnewi*R)*Const->rho_sea_water*um*um-Const->sub);
            x               = Xx[node];

            phe_data.val = erode_depth*Dx;
            phe_data.phe = PheBottom;

            get_phe_func( Const->get_phe_data , Xx[node] , &phe_data );

            // get_phe_func may have changed the erosion depth if there wasn't enough sediment.
            erode_depth = phe_data.val/Dx;

            for ( n=0 , av_graindensity = 0 , bulkdensitybottom = 0 ;
                  n<nGrains ;
                  n++ )
            {
               av_graindensity   += gDens[n]          * PheBottom[n];
               bulkdensitybottom += DepositDensity[n] * PheBottom[n];
            }
            porosity = 1.0 - bulkdensitybottom/av_graindensity;

/*
            profile_data = Consts.get_phe_data;

            EH_STRUCT_MEMBER( Sak_phe_query_t , &get_phe_query , x           ) = x;
            EH_STRUCT_MEMBER( Sak_phe_query_t , &get_phe_query , erode_depth ) = erode_depth;
            EH_STRUCT_MEMBER( Sak_phe_query_t , &get_phe_query , dx          ) = Dx;
            EH_STRUCT_MEMBER( Sak_phe_query_t , &get_phe_query , phe         ) = PheBottom;

            (*(Consts.get_phe))( (gpointer)&get_phe_query , profile_data );

            // the depth of erosion may change if there isn't enough sediment
            // available.
            erode_depth = EH_STRUCT_MEMBER( Sak_phe_query_t , &get_phe_query  , erode_depth );
*/

            erosion[i] = erode_depth
                       * PheBottom[i]*(1.-porosity)
                       / (Const->sua*S_SECONDS_PER_DAY);

//
// ... to here.
//
#endif

#if defined(YUSUKE)
// NOTE: commented out by eric.
            erosion[i] = (Const->c_drag* (1+ cnewi * R) * Const->rho_sea_water *um *um - Const->sub);
            erosion[i] *= PheBottom[i] * (1.0 - porosity);
            erosion[i] /= ( Const->sua * S_SECONDS_PER_DAY );
#endif

/*                      if (Lambda[i] * Dt >= 1)
                              fluxatbed = cnewi * HHnew[node]/Dt - LARGER(0, erosion[i]);
                        else
                              fluxatbed = Lambda[i] * cnewi * HHnew[node] - LARGER(0, erosion[i]);

                        if ( HHnew[node] <= smallh)
                              fluxatbed = HHnew[node]/Dt * cnewi - Stv[i] * erosion[i]; 
                        else
                              fluxatbed = Stv[i] * ( Ro * cnewi - erosion[i]);
*/
            if ( HHnew[node] <= smallh)
               fluxatbed = HHnew[node]/Dt * cnewi - eh_max( 0, erosion[i]); 
            else
               fluxatbed = Stv[i] *  Ro * cnewi - eh_max( 0, erosion[i]);

//                      if (fluxatbed * Dt + SEDMULTI[node][i] < 0) /* when no net erosion */
//                            fluxatbed = - SEDMULTI[node][i]/Dt;

            if (Xx[node] < DepositionStart )
               fluxatbed = 0.0;
#if defined( YUSUKE )
            if ( DEPTH[node] + fluxatbed*Dt/porosity/PheBottom[i] > -InitH)
               fluxatbed = 0.0;
#else
/*
            depth_node = get_depth( node ,
                                    Consts.get_depth ,
                                    Consts.depth_data );
*/
            depth_node = get_depth_func( Const->depth_data , Xx[node] );

            if ( depth_node + fluxatbed*Dt/porosity/PheBottom[i] > -InitH)
               fluxatbed = 0.0;
#endif

#if !defined(YUSUKE)
//
// NOTE: added by eric from here...
//
            dh = fluxatbed*Dt/porosity;

            sediment.t  = dh/Dx;
            sediment.id = i;
            if ( dh<0 ) remove_func( Const->remove_data , Xx[node] , &sediment );
            else        add_func   ( Const->add_data    , Xx[node] , &sediment );
/*
            if ( fluxatbed*Dt/porosity<0 )
            {
               erode_query.dh = fluxatbed*Dt/porosity;
               erode_query.i  = node;

               (*(Consts.remove))( (gpointer)&erode_query , Consts.remove_data );
            }
            else
            {
               add_query.dh = fluxatbed*Dt/porosity;
               add_query.i  = node;

               (*(Consts.add))( (gpointer)&add_query , Consts.add_data );
            }
*/
//
// ... to here.
//
#endif

            SEDMULTI[node][i] += fluxatbed * Dt /porosity;

#if defined(YUSUKE)
// NOTE: commented out by eric.  this is now changed in the add/remove routines.
            DEPTH[node] += fluxatbed * Dt /porosity;
#endif

            if (HHnew[node] < HMIN)
               CCMULTInew[node][i] = 0;
            else
               CCMULTInew[node][i] = cnewi - fluxatbed * Dt/ HHnew[node];
            
            if (CCMULTInew[node][i] < -HMIN)
            {
               eh_warning("negative CCMULTInew: t=%d, node=%d, grain=%d",count,node,i);
               eh_warning("cnew=%f, cold=%f, hnew=%f", CCMULTInew[node][i], cnewi, HHnew[node]);
               stopnumber = 1;
            }
            else if (CCMULTInew[node][i] < 0)
               CCMULTInew[node][i] = 0.0;
            CCMULTInew[node][i] = eh_max(0, CCMULTInew[node][i]);

            cnew += CCMULTInew[node][i];
            sedrate += fluxatbed;
         } //end of CCMULTI

         CCnew[node] = cnew;
         SED[node] += Dt * sedrate;
         SEDRATE[node] = sedrate;
         maxc = eh_max(maxc,cnew);
         totalsusp += cnew * HHnew[node];
      } /*end STEP2*/
      
      if (maxc > HMIN && totalsusp > HMIN)
         ;
      else
      {
         eh_warning("maxc=%f, totalsusp=%f", maxc, totalsusp);
         eh_warning("ccmultinew=%f, hew=%f",CCMULTInew[node-1][i-1], HHnew[node-1]);
         stopnumber = 1;
      }
      
      //free outflow at downstream end
      if (xhead <= Basin_Len)
      {
         outflow_unew = 0;
         outflow_cnew = 0;
         outflow_hnew = 0;
      }
      else if (xhead <= Basin_Len +Dx)
      {
         outflow_unew = 0; 
         outflow_hnew = outflow_h + HH[nNodes-2] * U[nNodes-1]/Dx;
         outflow_cnew = CC[nNodes-2];
      }
      else
      {
         outflow_unew = U[nNodes-1];
         outflow_hnew = HH[nNodes-2];
         outflow_cnew = CC[nNodes-2];
      }

      /* STEP3: calculate new velocity*/
      /* trying to use variables at t + 0.5 Dt */
      for (node = 1; node <= headnode; node++)
      {
         u = Utemp[node];
#if defined(YUSUKE)
         s = - sin( atan( (DEPTH[node] - DEPTH[node-1])/Dx ));
#else
         // get the depths of the i-1 and i nodes from the sakura architecture
         // so that we can calculate the slope.
/*
         depth_0 = get_depth( node-1 , Consts.get_depth , Consts.depth_data );
         depth_1 = get_depth( node   , Consts.get_depth , Consts.depth_data );
*/
         depth_0 = get_depth_func( Const->depth_data , Xx[node-1] );
         depth_1 = get_depth_func( Const->depth_data , Xx[node]   );

         s       = - sin( atan( (depth_1-depth_0)/Dx ) );
#endif
         ustar = sqrt(Const->c_drag) * Utemp[node];

         ul = Utemp[node-1]; 
         cl = 0.5 * (CC[node-1] + CCnew[node-1] );
         hl = 0.5 * (HH[node-1] + HHnew[node-1]);

         if (node == nNodes-1)
         {
            cr = 0.5 * (outflow_c + outflow_cnew);
            hr = 0.5 * (outflow_h + outflow_hnew);
            ur = outflow_unew;
            urr = outflow_unew;
         }
         else if (node == nNodes-2)
         {
            cr = 0.5 * (CC[node] + CCnew[node] );
            hr = 0.5 * (HH[node] + HHnew[node]);
            ur = Utemp[node+1];
            urr = outflow_unew;
         }
         else
         {
            cr = 0.5 * (CC[node] + CCnew[node]);
            hr = 0.5 * (HH[node] + HHnew[node]);
            ur = Utemp[node+1];
            urr = Utemp[node+2];
         }
         
         if (node == 1)
            ull = inflow_u;
         else
            ull = Utemp[node-2];
         cm = 0.5 * ( cl + cr );
         hm = 0.5 * ( hl + hr );
         
/*
          if (hm < HMIN){
             fprintf(stderr, "hm too small in STEP3 at count=%d, node=%d, headnode=%d\n", count, node, headnode);
             stopnumber = 1;
          }
*/


         // compute entrainment 
         Ew = 0.0;
         if (u != 0.0)
         {
            Ri = R * G * cm * hm / eh_sqr(u);
            Ew = Const->e_a / (Const->e_b + Ri);
         }
         
         
         Unew[node] = U[node] + dudt( u, ul, ur, ull, urr, hl, hr, hm, cl, cr, cm, ustar, s, Ew, smallh, Dx, Const->c_drag, Const->mu_water) * Dt;
         if (fabs(Unew[node])  > UPPERLIMIT)
         {
            eh_warning("too fast at %d",node);
            stopnumber = 1;
         }
         
      } /*end of STEP3*/
      
      if (xhead <= Basin_Len)
         Unew[nNodes-1] = 0;
      else
      {
         Unew[nNodes-1] = Unew[nNodes-1] - Dt/Dx*Unew[nNodes-2] * (Unew[nNodes-1] - Unew[nNodes-2]);                    };

      /* velocity at flow head boundary */
      xhead += uhead * Dt;
      headnode = (int)floor(xhead/Dx);
      if (headnode == nNodes-1)
      {
         //fprintf(stderr,"flow reaches downstream end\n");
         stopnumber = 0;
      }
      
      /* update variables */
      for (node = 0; node < nNodes-1; node++)
      {
         U[node] = Unew[node];
         HH[node] = HHnew[node];
         CC[node] = CCnew[node];
         for (i = 0; i < nGrains; i++)
            CCMULTI[node][i] = CCMULTInew[node][i];
      }
      U[node] = Unew[node];
      maxu = eh_max(maxu, Unew[node]);
      xtravel[count] = xhead;

   }

   for (node=0; node <nNodes-1; node++)
      SED[node] = SED[node] + CC[node] * HH[node];
   
   for (node=0; node <nNodes-1; node++)
   {
      for (i=0; i <nGrains; i++)
      {
         //Deposit[i][node] = SEDMULTI[node][i] + CCMULTI[node][i] * HH[node];
         //Deposit[i][node] = (SEDMULTI[node][i] + SEDMULTI[node-1][i])*0.5;;
         Deposit[i][node] = SEDMULTI[node][i];
      }
   }

//   outputData(Outfp2, nNodes, time, U, HH, CC, SED, Utemp, SEDRATE, xhead, uhead, headnode);

   if ( fp_data )
   {
      fwrite( Xx  , nNodes-1 , sizeof(double) , fp_data );
      fwrite( U   , nNodes-1 , sizeof(double) , fp_data );
      fwrite( HH  , nNodes-1 , sizeof(double) , fp_data );
      fwrite( CC  , nNodes-1 , sizeof(double) , fp_data );
      fwrite( SED , nNodes-1 , sizeof(double) , fp_data );
   }

   eh_debug("END...total time = %.1f sec xhead = %f m", time, xhead);
   eh_debug("maxc=  %f; totalsusp= %f, maxu = %f, initH=%f", maxc, totalsusp, maxu,InitH);
   
   // de-allocate memory    */
   eh_free( PheBottom );
   eh_free( U       );
   eh_free( HH      );
   eh_free( CC      );
   eh_free( SED     );
   eh_free( SEDRATE );
   eh_free( Utemp   );
   eh_free( Unew    );
   eh_free( HHnew   );
   eh_free( CCnew   );
   eh_free( xtravel );
   eh_free( erosion );   

   eh_free_2( CCMULTI    );
   eh_free_2( CCMULTInew );
   eh_free_2( SEDMULTI   ); 

   return TRUE;
}

/** Write sakura output to a binary file.

@param fp      A pointer to a FILE.
@param n_nodes The number of nodes.
@param x       Array of node positions.
@param u       Array of node velocities.
@param h       Array of node thicknesses.
@param c       Array of node concentrations.
@param dz      Array of changes in node elevation.

*/
void output_data( FILE* fp , int n_nodes , double *x , double *u , double *h , double *c , double *dz )
{
   fwrite( x  , n_nodes , sizeof(double) , fp );
   fwrite( u  , n_nodes , sizeof(double) , fp );
   fwrite( h  , n_nodes , sizeof(double) , fp );
   fwrite( c  , n_nodes , sizeof(double) , fp );
   fwrite( dz , n_nodes , sizeof(double) , fp );
}

/** Get a node depth from the parent architecture.

The architecture that sakura is being run in may change.  For instance, the
standalone version of sakura will have one architecture while sedflux may
have another.  The Eh_query_func that is passed, is an architecture specific
function that will get a variable from the appropriate architecture.  The
bathy_data pointer will point to architecture dependent bathymetry data that
will be queried.

@param i          An index to a node.
@param get_depth  An Eh_query_func to get a depth value from an architecture.
@param bathy_data A pointer to bathymetry data.

@return The node depth.
*/
/*
double get_depth( int i , Eh_query_func get_depth , gpointer bathy_data )
{
   Sak_depth_query_t depth_query;
   double depth;

   // set the index to query.
   EH_STRUCT_MEMBER( Sak_depth_query_t , &depth_query , i ) = i;

   // call the query function.  the depth is set within the depth_query
   // structure.
   (*get_depth)( (gpointer)&depth_query , bathy_data );

   // get the depth from the depth_query structure.
   depth = EH_STRUCT_MEMBER( Sak_depth_query_t , &depth_query , depth );

   return depth;
}
*/
