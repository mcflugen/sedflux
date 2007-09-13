//--- // // This file is part of sedflux.
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
static gboolean
step_1( double* x , double* u , double* c , double* h , gint len , double dt , double* u_new , Sakura_const_st* Const ,
        gint head_node , double inflow_u , double outflow_u , double outflow_c , double outflow_h );

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
#define NEW

#if !defined(NEW)
gboolean
sakura( double Dx         , double Dt              , double Basin_Len ,
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
/*
         if ( standalone ) inflow_u = Init_U[count];
         else              inflow_u = Init_U[0];
         if ( standalone ) inflow_c = Init_C[count];
         else              inflow_c = Init_C[0];
*/
         inflow_u = Init_U[0];
         inflow_c = Init_C[0];
         U[0]     = inflow_u;
         Utemp[0] = inflow_u;
         inflow_h = InitH; //HH[0] = inflow_h;
         //  inflow_c = (discharge_density[count] - Consts.rhoRW)/(av_graindensity - Consts.rhoRW);
      }
      else
      {
         inflow_u = 0;
         U[0]     = inflow_u;
         Utemp[0] = inflow_u;
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

      step_1( Xx , U , CC , HH , nNodes , Dt , Utemp , Const , headnode , inflow_u , outflow_u , outflow_c , outflow_h );
   
#if defined( IGNORE )
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
#endif

      uhead_1 = uhead;   
      uhead = eh_max(Utemp[node-1], Utemp[node-2]);

      if      ( node  <= 1) uhead = Utemp[0];
      else if ( uhead >  0) uhead = eh_min(uhead, 1.5 * pow(G * R * cl * hl * uhead, 0.3333));
      else                  uhead = eh_max(Utemp[node-1], Utemp[node-2]);

      //if ( uhead > 0 )
      //   ;
      //else
      if ( uhead<=0 )
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

            phe_data.val      = erode_depth*Dx;
            phe_data.phe      = PheBottom;
            phe_data.n_grains = nGrains;

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
      
      //if (maxc > HMIN && totalsusp > HMIN)
      //   ;
      //else
      if ( maxc<=HMIN || totalsusp<=HMIN )
      {
         eh_warning("maxc=%f, totalsusp=%f", maxc, totalsusp);
         eh_warning("ccmultinew=%f, hew=%f",CCMULTInew[node-1][i-1], HHnew[node-1]);
         eh_watch_int( node );
         eh_watch_dbl( HMIN );
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
#endif

#if !defined(NEW)
gboolean
step_1( double* x , double* u , double* c , double* h , gint len , double dt , double* u_new , Sakura_const_st* Const ,
        gint head_node , double inflow_u , double outflow_u , double outflow_c , double outflow_h )
{
   gboolean success = TRUE;

   eh_require( x );
   eh_require( c );
   eh_require( h );
   eh_require( u );
   eh_require( u_new );

   if ( x && c && h && u && u_new )
   {
      gint i;
      double dx = x[1] - x[0];
      double u_star;
      double u_0, ul, ull, ur, urr;
      double cl, cm, cr, hl, hm, hr;
      double depth_0, depth_1;
      double s;
      Sakura_get_func get_depth_func = Const->get_depth;

      // STEP 1: calculate tentative velocity at t + 0.5Dt
      // start from node =1 because velocity is given at upstream end (node=0)
      // calculate only within the flow (behind the head position)
      head_node = eh_min(head_node, len-1);
      //smallh   = Stv[0] * dt * 2; 

      for ( i=1 ; i<=head_node ; i++ )
      {
         u_0   = u[i];

#if defined(YUSUKE)
         s     = - sin( atan( (DEPTH[i] - DEPTH[-1])/dx ));
#else
         // get the depths of the i-1 and i nodes from the sakura architecture
         // so that we can calculate the slope.
/*
         depth_0 = get_depth( node-1 , Consts.get_depth , Consts.depth_data );
         depth_1 = get_depth( node   , Consts.get_depth , Consts.depth_data );
*/

         depth_0 = get_depth_func( Const->depth_data , x[i-1] );
         depth_1 = get_depth_func( Const->depth_data , x[i]   );

         s       = - sin( atan( (depth_1-depth_0)/dx ) );
#endif
         //ustar = sqrt(Const->c_drag) * u;

         ul = u[i-1]; 
         cl = c[i-1]; 
         hl = h[i-1]; 

         if ( i==len-1 )
         {
            cr  = outflow_c;
            hr  = outflow_h;
            ur  = outflow_u;
            urr = outflow_u;
         }
         else if ( i==len-2)
         {
            cr  = c[i];
            hr  = h[i];
            ur  = u[i+1];
            urr = outflow_u;
         }
         else
         {
            cr  = c[i];
            hr  = h[i];
            ur  = u[i+1];
            urr = u[i+2];
         }
         
         if ( i==1) ull = inflow_u;
         else       ull = u[i-2];

         // value at the node calculated from those at midpoints
         cm = 0.5 * ( cl + cr );
         hm = 0.5 * ( hl + hr );
         
         //if (hm == 0)
         //   ;
         //else if (hm < HMIN)
         if ( hm>0 && hm<HMIN )
         {
            eh_warning( "hm too small in STEP 1 at node = %d" , i );
            success = FALSE;
         }

         // compute water entrainment 
/*
         Ew = 0.0;
         if ( !eh_compare_dbl(u,0.,1e-12) )
         {
            Ri = R * G * hm / eh_sqr(u);
            Ew = Const->e_a / (Const->e_b + Ri);
         }
*/
                 
         // tentative varibales with dt = 0.5 Dt 
         u_new[i] = u_0
                  +   dudt( u_0 , ul , ur     , ull , urr           , hl    ,
                            hr  , hm , cl     , cr  , cm            , -9999 ,
                            s   , -9 , -99999 , dx  , Const->c_drag , Const->mu_water )
                    * dt * 0.5;
         
      } // end of STEP1
   }
   else
      success = FALSE;

   return success;
}

gboolean
step_3( double* u , gint len , double* u_new )
{
   gboolean success = TRUE;

   eh_require( u );

   if ( u )
   {
      gint i;
      double dx = x[1] - x[0];
      double u_star;
      double u_0, ul, ull, ur, urr;
      double cl, cm, cr, hl, hm, hr;

      /* STEP3: calculate new velocity*/
      /* trying to use variables at t + 0.5 Dt */
      for ( i=1 ; i<=headnode ; i++ )
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
         depth_0 = get_depth_func( Const->depth_data , x[i-1] );
         depth_1 = get_depth_func( Const->depth_data , x[i]   );

         s       = - sin( atan( (depth_1-depth_0)/dx ) );
#endif
         //ustar = sqrt(Const->c_drag) * Utemp[node];

         ul = Utemp[i-1]; 
         cl = 0.5 * (c[i-1] + c_new[i-1] );
         hl = 0.5 * (h[i-1] + h_new[i-1]);

         if ( i==len-1)
         {
            cr  = 0.5 * (outflow_c + outflow_cnew);
            hr  = 0.5 * (outflow_h + outflow_hnew);
            ur  = outflow_unew;
            urr = outflow_unew;
         }
         else if ( i==len-2)
         {
            cr  = 0.5 * (c[i] + c_new[i] );
            hr  = 0.5 * (h[i] + h_new[i]);
            ur  = Utemp[i+1];
            urr = outflow_unew;
         }
         else
         {
            cr  = 0.5 * (c[i] + c_new[i]);
            hr  = 0.5 * (h[i] + h_new[i]);
            ur  = Utemp[i+1];
            urr = Utemp[i+2];
         }
         
         if ( i==1) ull = inflow_u;
         else       ull = Utemp[node-2];

         cm = 0.5 * ( cl + cr );
         hm = 0.5 * ( hl + hr );
         
/*
          if (hm < HMIN){
             fprintf(stderr, "hm too small in STEP3 at count=%d, node=%d, headnode=%d\n", count, node, headnode);
             stopnumber = 1;
          }
*/

/*
         // compute entrainment 
         Ew = 0.0;
         if (u != 0.0)
         {
            Ri = R * G * cm * hm / eh_sqr(u);
            Ew = Const->e_a / (Const->e_b + Ri);
         }
*/
         
         
         u_new[i] = U[i]
                  + dudt( u, ul, ur, ull, urr,
                          hl, hr, hm,
                          cl, cr, cm,
                          -999 , s, -999 , -999 , Dd, Const->c_drag, Const->mu_water) * dt;

         if (fabs(Unew[node])  > UPPERLIMIT)
         {
            eh_warning("too fast at %d",node);
            successs = FALSE;
         }
      }
   }
   else
      success = FALSE;

   return success;
}

#endif

#if defined( NEW )

Sakura_sediment*
sakura_sediment_new( gint n_grains )
{
   Sakura_sediment* s = NULL;

   if ( n_grains>0 )
   {
      s = eh_new( Sakura_sediment , 1 );
      s->rho_grain  = eh_new0( double , n_grains );
      s->rho_dep    = eh_new0( double , n_grains );
      s->u_settling = eh_new0( double , n_grains );

      s->len = n_grains;
   }

   return s;
}

Sakura_sediment*
sakura_sediment_destroy( Sakura_sediment* s )
{
   if ( s )
   {
      eh_free( s->rho_grain  );
      eh_free( s->rho_dep    );
      eh_free( s->u_settling );
      eh_free( s );
   }
   return NULL;
}

Sakura_sediment*
sakura_sediment_set_rho_grain( Sakura_sediment* s , double* x )
{
   if ( s && x )
   {
      eh_dbl_array_copy( s->rho_grain , x , s->len );
   }
   return s;
}

Sakura_sediment*
sakura_sediment_set_rho_dep( Sakura_sediment* s , double* x )
{
   if ( s && x )
   {
      eh_dbl_array_copy( s->rho_dep , x , s->len );
   }
   return s;
}

Sakura_sediment*
sakura_sediment_set_u_settling( Sakura_sediment* s , double* x )
{
   if ( s && x )
   {
      eh_dbl_array_copy( s->u_settling , x , s->len );
   }
   return s;
}

Sakura_array*
sakura_array_new( gint len , gint n_grain )
{
   Sakura_array* a = NULL;

   if ( len>0 )
   {
      gint n_nodes = len + 4;
      gint i;

      a = eh_new( Sakura_array , 1 );

      a->x = eh_new0( double , n_nodes ) + 2;
      a->w = eh_new0( double , n_nodes ) + 2;
      a->h = eh_new0( double , n_nodes ) + 2;
      a->u = eh_new0( double , n_nodes ) + 2;
      a->c = eh_new0( double , n_nodes ) + 2;

      a->c_grain     = eh_new0( double* , n_nodes ) + 2;
      a->c_grain[-2] = eh_new0( double  , n_nodes*n_grain );
      for ( i=-1 ; i<len+2 ; i ++ )
         a->c_grain[i] = a->c_grain[i-1] + n_grain;

      a->d     = eh_new0( double* , n_nodes ) + 2;
      a->d[-2] = eh_new0( double  , n_nodes*n_grain );
      for ( i=-1 ; i<len+2 ; i ++ )
         a->d[i] = a->d[i-1] + n_grain;

      a->e     = eh_new0( double* , n_nodes ) + 2;
      a->e[-2] = eh_new0( double  , n_nodes*n_grain );
      for ( i=-1 ; i<len+2 ; i ++ )
         a->e[i] = a->e[i-1] + n_grain;

      a->len     = len;
      a->n_grain = n_grain;
   }

   return a;
}

Sakura_array*
sakura_array_destroy( Sakura_array* a )
{
   if ( a )
   {
      gint i;

      a->x -= 2;
      a->w -= 2;
      a->h -= 2;
      a->u -= 2;
      a->c -= 2;

      eh_free( a->x );
      eh_free( a->w );
      eh_free( a->h );
      eh_free( a->u );
      eh_free( a->c );

      a->c_grain -= 2;
      a->d       -= 2;
      a->e       -= 2;

      eh_free( a->c_grain[0] );
      eh_free( a->c_grain    );
      eh_free( a->d[0]       );
      eh_free( a->d          );
      eh_free( a->e[0]       );
      eh_free( a->e          );

      eh_free( a );
   }
   return NULL;
}

Sakura_array*
sakura_array_copy( Sakura_array* d , Sakura_array* s )
{
   if ( s )
   {
      if ( !d )
         d = sakura_array_new( s->len , s->n_grain );

      eh_dbl_array_copy( d->x-2 , s->x-2 , s->len+4 );
      eh_dbl_array_copy( d->w-2 , s->w-2 , s->len+4 );
      eh_dbl_array_copy( d->h-2 , s->h-2 , s->len+4 );
      eh_dbl_array_copy( d->u-2 , s->u-2 , s->len+4 );
      eh_dbl_array_copy( d->c-2 , s->c-2 , s->len+4 );

      eh_dbl_array_copy( d->c_grain[-2] , s->c_grain[-2] , (s->len+4)*s->n_grain );
      eh_dbl_array_copy( d->d[-2]       , s->d[-2]       , (s->len+4)*s->n_grain );
      eh_dbl_array_copy( d->e[-2]       , s->e[-2]       , (s->len+4)*s->n_grain );
   }

   return d;
}

Sakura_array*
sakura_array_set_x( Sakura_array* a , double* x )
{
   eh_require( a    );
   eh_require( a->x );
   eh_require( x    );

   if ( a && x )
   {
      double dx;

      eh_dbl_array_copy( a->x , x , a->len );

      dx = x[1] - x[0];

      eh_require( dx>0 );

      a->x[-1] = x[0] - dx;
      a->x[-2] = x[0] - dx*2.;

      dx = x[a->len-1] - x[a->len-2];

      eh_require( dx>0 );

      a->x[a->len  ] = x[a->len-1] + dx;
      a->x[a->len+1] = x[a->len-1] + dx*2.;
   }

   return a;
}

Sakura_array*
sakura_array_set_w( Sakura_array* a , double* w )
{
   eh_require( a    );
   eh_require( a->w );
   eh_require( w    );

   if ( a && w )
   {
      eh_dbl_array_copy( a->w , w , a->len );

      a->w[-1] = w[0];
      a->w[-2] = w[0];

      a->w[a->len  ] = w[a->len-1];
      a->w[a->len+1] = w[a->len-1];
   }

   return a;
}

Sakura_array*
sakura_array_set_bc( Sakura_array* a , Sakura_node* inflow , Sakura_node* outflow )
{
   if ( a )
   {
      const gint len = a->len;

      a->u[0]     = inflow->u;
      a->u[-1]    = inflow->u;
      a->u[-2]    = inflow->u;

      a->c[-1]    = inflow->c;
      a->c[-2]    = inflow->c;

      eh_dbl_array_copy( a->c_grain[-1] , inflow->c_grain , a->n_grain );
      eh_dbl_array_copy( a->c_grain[-2] , inflow->c_grain , a->n_grain );

      a->h[-1]    = inflow->h;
      a->h[-2]    = inflow->h;

      a->u[len]   = outflow->u;
      a->u[len+1] = outflow->u;

      a->c[len]   = outflow->c;
      a->c[len+1] = outflow->c;

      eh_dbl_array_copy( a->c_grain[len]   , outflow->c_grain , a->n_grain );
      eh_dbl_array_copy( a->c_grain[len+1] , outflow->c_grain , a->n_grain );

      a->h[len]   = outflow->h;
      a->h[len+1] = outflow->h;
   }
   return a;
}

double
sakura_array_mass_in_susp( Sakura_array* a , Sakura_sediment* s )
{
   double mass = 0.;

   eh_require( a );
   eh_require( s );

   if ( a && s )
   {
      gint   i;
      gint   n;
      double vol_w;

      for ( i=0 ; i<a->len ; i++ )
      {
         vol_w = a->h[i]*a->w[i]*(a->x[i+1]-a->x[i]);
         for ( n=0 ; n<s->len ; n++ )
            mass += vol_w*a->c_grain[i][n]*s->rho_grain[n];
      }
   }

   return mass;
}

double
sakura_array_mass_eroded( Sakura_array* a , Sakura_sediment* s )
{
   double mass = 0.;

   eh_require( a );
   eh_require( s );

   if ( a && s )
   {
      gint   i;
      gint   n;

      for ( i=0 ; i<a->len ; i++ )
         for ( n=0 ; n<s->len ; n++ )
            mass += a->e[i][n]*s->rho_grain[n];
   }

   return mass;
}

double
sakura_array_mass_deposited( Sakura_array* a , Sakura_sediment* s )
{
   double mass = 0.;

   eh_require( a );
   eh_require( s );

   if ( a && s )
   {
      gint   i;
      gint   n;
      double vol_w;

      for ( i=0 ; i<a->len ; i++ )
         for ( n=0 ; n<s->len ; n++ )
            mass += a->d[i][n]*s->rho_grain[n];
   }

   return mass;
}

Sakura_node*
sakura_node_new( double u , double c , double h , double* c_grain , gint len )
{
   return sakura_node_set( NULL , u , c , h , c_grain , len );
}

Sakura_node*
sakura_node_destroy( Sakura_node* x )
{
   if ( x )
   {
      eh_free( x->c_grain );
      eh_free( x );
   }
   return NULL;
}

Sakura_node*
sakura_node_set( Sakura_node* x , double u , double c , double h , double* c_grain , gint len )
{
   if ( !x ) x = eh_new( Sakura_node , 1 );

   if ( x )
   {
      x->u = u;
      x->c = c;
      x->h = h;

      if ( len!=x->n_grain )
      {
         eh_free( x->c_grain );
         x->c_grain = NULL;
      }

      if      ( c_grain    ) x->c_grain = eh_dbl_array_copy( x->c_grain , c_grain , len );
      else if ( x->c_grain ) x->c_grain = eh_dbl_array_set ( x->c_grain , len , 0. );
      else                   x->c_grain = eh_new0          ( double , len );

      x->n_grain = len;

   }

   return x;
}
      
gboolean
sakura_set_outflow( Sakura_node* out , Sakura_array* a , double x_head , double dt , double dx )
{
   gboolean success = TRUE;

   eh_require( out );
   eh_require( a   );

   if ( out && a )
   {
      double basin_len = a->x[a->len-1] - a->x[0];
      gint   n_grains  = a->n_grain;
      gint   n_nodes   = a->len;
      double  u = 0.; // Free outflow at downstream end
      double  c = 0.;
      double  h = 0.;
      double* c_grain = eh_new0( double , n_grains );

      //if ( x_head > basin_len+dx )
      if ( x_head > a->x[a->len-1]+dx )
      {
         u = a->u[n_nodes-1];
         h = a->h[n_nodes-2];
         c = a->c[n_nodes-2];
         eh_dbl_array_copy( c_grain , a->c_grain[n_nodes-2] , n_grains );
      }
      //else if ( x_head > basin_len )
      else if ( x_head > a->x[a->len-1] )
      {
         u = 0; 
         h = out->h + a->h[n_nodes-2] * a->u[n_nodes-1] * dt/dx;
         c = a->c[n_nodes-2];
         eh_dbl_array_copy( out->c_grain , a->c_grain[n_nodes-2] , n_grains );
      }

      sakura_node_set( out , u , c , h , c_grain , n_grains );

      eh_free( c_grain );
   }

   return success;
}

double
sakura_get_sin_slope( Sakura_get_func f , gpointer data , Sakura_array* a , gint i )
{
   double s = 0;

   eh_require( f );
if ( i<1 ) i=1;

   if ( f )
   {
      double depth_0 = f( data , a->x[i-1] );
      double depth_1 = f( data , a->x[i]   );
      double dx      = a->x[i] - a->x[i-1];

      s = -sin( atan( (depth_1-depth_0)/dx ) );
   }
   return s;
}

// This is step 1
gboolean
calculate_mid_vel( Sakura_array* a_mid , Sakura_array* a , gint ind_head , Sakura_const_st* con )
{
   gboolean success = TRUE;

   eh_require( a_mid );
   eh_require( a     );
   eh_require( con   );

   if ( a_mid && a && con )
   {
      gint i;
      double dx = a->x[1] - a->x[0];
      double u_star;
      double u_head;
      double u_0, ul, ull, ur, urr;
      double cl, cm, cr, hl, hm, hr;
      double s;
      double du_dt;
      const double    dt             = con->dt;
      Sakura_get_func get_depth_func = con->get_depth;
      double *x     = a->x;
      double *u     = a->u;
      double *h     = a->h;
      double *c     = a->c;
      double *u_new = a_mid->u;

      // STEP 1: calculate tentative velocity at t + 0.5Dt
      // start from node =1 because velocity is given at upstream end (node=0)
      // calculate only within the flow (behind the head position)
      ind_head = eh_min( ind_head , a_mid->len-1);

      for ( i=1 ; i<=ind_head ; i++ )
      {
         u_0   = u[i];

         // get the depths of the i-1 and i nodes from the sakura architecture
         // so that we can calculate the slope.
         s = sakura_get_sin_slope( con->get_depth , con->depth_data , a , i );
/*
         depth_0 = get_depth_func( Const->depth_data , x[i-1] );
         depth_1 = get_depth_func( Const->depth_data , x[i]   );
         s       = - sin( atan( (depth_1-depth_0)/dx ) );
*/

         ull = u[i-2];
         ul  = u[i-1]; 
         ur  = u[i+1];
         urr = u[i+2];

         cl  = c[i-1]; 
         cr  = c[i];

         hl  = h[i-1]; 
         hr  = h[i];

         // value at the node calculated from those at midpoints
         cm = 0.5 * ( cl + cr );
         hm = 0.5 * ( hl + hr );
         
         if ( hm>0 && hm<HMIN )
         {
            eh_warning( "hm too small in STEP 1 at node = %d" , i );
            success = FALSE;
         }
         du_dt = dudt( u_0 , ul , ur     , ull , urr       , hl    ,
                       hr  , hm , cl     , cr  , cm        , -9999 ,
                       s   , -9 , -99999 , dx  , con->c_drag , con->mu_water );
         // tentative variables with dt = 0.5 Dt 
         u_new[i] = u_0 + du_dt * dt * .5;

      } // end of STEP1

   }
   else
      success = FALSE;

   return success;
}

// This is step 3
gboolean
calculate_next_vel( Sakura_array* a_last , Sakura_array* a_mid , Sakura_array* a_next , gint ind_head , Sakura_const_st* Const )
{
   gboolean success = TRUE;

   eh_require( a_last );
   eh_require( a_mid  );
   eh_require( a_next );

   if ( a_last && a_mid && a_next )
   {
      const double dx = a_last->x[1] - a_last->x[0];
      const double dt = Const->dt;
      gint i;
      double u_star;
      double u_0, ul, ull, ur, urr;
      double cl, cm, cr, hl, hm, hr;
      double* u_mid  = a_mid->u;
      double* c_mid  = a_mid->c;
      double* h_mid  = a_mid->h;
      double* u_last = a_last->u;
      double* u_next = a_next->u;
      double s;

      ind_head = eh_min( ind_head , a_last->len-1 );

      /* STEP3: calculate new velocity*/
      /* trying to use variables at t + 0.5 Dt */
      for ( i=1 ; i<=ind_head ; i++ )
      {
         // get the depths of the i-1 and i nodes from the sakura architecture
         // so that we can calculate the slope.
/*
         depth_0 = get_depth_func( Const->depth_data , a_last->x[i-1] );
         depth_1 = get_depth_func( Const->depth_data , a_last->x[i]   );
         s       = - sin( atan( (depth_1-depth_0)/dx ) );
*/

         s = sakura_get_sin_slope( Const->get_depth , Const->depth_data , a_last , i );

// u_temp is at t+.5dt
         ull = u_mid[i-2];
         ul  = u_mid[i-1]; 
         u_0 = u_mid[i];
         ur  = u_mid[i+1];
         urr = u_mid[i+2];

// c is at t?
// c_new is at t+dt?
         cl  = c_mid[i-1];
         cr  = c_mid[i];

         hl  = h_mid[i-1];
         hr  = h_mid[i];

         cm = 0.5 * ( cl + cr );
         hm = 0.5 * ( hl + hr );
         
// U is at t?
         u_next[i] = u_last[i]
                   + dudt( u_0, ul, ur, ull, urr,
                           hl, hr, hm,
                           cl, cr, cm,
                           -999 , s, -999 , -999 , dx, Const->c_drag, Const->mu_water) * dt;

         eh_require( fabs(u_next[i])<=UPPERLIMIT );

         if (fabs(u_next[i]) > UPPERLIMIT)
         {
            eh_warning("too fast at %d",i);
            success = FALSE;
         }
      }
/*
      if (x_head <= basin_len) u_new[n_nodes-1]  = 0;
      else                     u_new[n_nodes-1] -= dt/dx*u_new[n_nodes-2] * (u_new[n_nodes-1] - u_new[n_nodes-2]);

      // velocity at flow head boundary
      x_head   += u_head * dt;
      ind_head  = (int)floor(x_head/dx);
      if ( ind_head == n_nodes-1)
      {
         //fprintf(stderr,"flow reaches downstream end\n");
         stopnumber = 0;
      }
*/
   }
   else
      success = FALSE;

   return success;
}


double
sakura_erode_depth( double rho_f , double u , double dt , double sua , double sub , double c_drag )
{
   double e = 0;

   eh_require( rho_f>=0 );
   eh_require( u>0      );
   eh_require( dt>0     );

   if ( dt>0 )
   {
      // Amount of erosion in m
      e = ( c_drag * rho_f * u*u - sub ) / sua * ( dt * S_DAYS_PER_SECOND );

      if ( e<0 ) e = 0;
   }

   return e;
}

Sakura_array*
sakura_next_c_grain( Sakura_array* a_next , Sakura_array* a_last , double* u , gint i , double dt , Sakura_sediment* sed )
{
   eh_require( a_next );
   eh_require( a_last );
   eh_require( sed    );

   if ( a_next && a_last && sed )
   {
      gint n;
      double cll, cl, c_0, cr, crr;
      double ul, ur;
      double wl, wr;
      double hll, hl, h_0, hr, hrr;
      double small_h;
      double c_grain_new;
      double df_dt;
      const gint   n_grain = a_last->n_grain;
      const double dx      = a_last->x[i+1] - a_last->x[i];

      ul = u[i];
      ur = u[i+1];

      wl = a_last->w[i];
      wr = a_last->w[i+1]; 

      hll = a_last->h[i-2];
      hl  = a_last->h[i-1];
      h_0 = a_last->h[i  ];
      hr  = a_last->h[i+1];
      hrr = a_last->h[i+2];

      for ( n = 0 ; n<n_grain ; n++)
      {
         small_h = sed->u_settling[n] * dt * Ro;

         cll = a_last->c_grain[i-2][n];
         cl  = a_last->c_grain[i-1][n];
         c_0 = a_last->c_grain[i  ][n];
         cr  = a_last->c_grain[i+1][n];
         crr = a_last->c_grain[i+2][n];

         eh_require( a_next->h[i]>=HMIN );
            
         if ( a_next->h[i]<HMIN )
         {
               c_grain_new = 0;
               //stopnumber = 1;
         }
         else
         {
            df_dt = dfdt(ul, ur, wl, wr, hl*cl, hr*cr, hll*cll, hrr*crr, h_0*c_0, dx, 0);
            c_grain_new = (c_0 * h_0 + dt * df_dt )/a_next->h[i];

            if ( c_grain_new < -HMIN)
            {
               eh_warning("negative new CC: node= %d, i= %d", i, n);
               eh_warning("cnew= %f, cold=%f", c_grain_new, c_0);
               c_grain_new = 0;
            }
         } //cnewi is the new concentration due to sediment transport by the flow

         a_next->c_grain[i][n] = c_grain_new;
      }

      a_next->c[i] = eh_dbl_array_sum( a_next->c_grain[i] , n_grain );

   }
   else
      a_next = NULL;

   return a_next;
}

double
sakura_rho_flow( double* c_grain , double* rho_grain , gint n_grains , double rho_water )
{
   double rho_f = 0;

   {
      gint n;
      for ( n=0 ; n<n_grains ; n++ )
         rho_f += c_grain[n]*rho_grain[n];

      rho_f += rho_water;
   }

   return rho_f;
}

double
sakura_erode( Sakura_array* a , Sakura_sediment* sed , double* u , gint i , double dt , Sakura_const_st* c )
{
   double ero = 0;

   eh_require( a    );
   eh_require( sed  );
   eh_require( c    );
   eh_require( dt>0 );

   if ( a && sed && c && dt>0 )
   {
      gint           n;
      const gint     n_grains   = sed->len;
      const double   dx         = a->x[i+1] - a->x[i];
      double*        phe_bottom = eh_new( double , n_grains );
      const double   vol_w      = dx*a->w[i]*a->h[i];
      double         rho_f;
      double         e_tot;
      double         e_grain;
      double         p;
      Sakura_cell_st sediment;
      Sakura_phe_st  phe_data;

      // Density of the flow
      rho_f      = sakura_rho_flow( a->c_grain[i] , sed->rho_grain , n_grains , c->rho_sea_water );

      // Total erosion depth (in meters of sediment plus water) over the time step
      e_tot      = sakura_erode_depth( rho_f , .5*(u[i]+u[i+1]) , dt , c->sua , c->sub , c->c_drag );

      // Cubic meters of eroded sediment plus water
      phe_data.val      = e_tot*dx*a->w[i];
      phe_data.phe      = phe_bottom;
      phe_data.n_grains = n_grains;

      c->get_phe( c->get_phe_data , a->x[i] , &phe_data );

      // get_phe_func may have changed the erosion depth if there wasn't enough sediment.
      // meters of sediment plus water
      e_tot = phe_data.val/(dx*a->w[i]);

      for ( n=0 ; n<n_grains ; n++ )
      {
//         porosity = ( sed->rho_grain[n] - sed->rho_dep[n] ) / ( sed->rho_grain[n] - rho_sea_water );
//         e_grain = e_tot*phe_bottom[n]*(1.-porosity);

         p = ( sed->rho_grain[n] - sed->rho_dep[n] ) / ( sed->rho_grain[n] - c->rho_sea_water );

         // Meters of sediment plus water
         e_grain = e_tot*phe_bottom[n];

         // Cubic meters of sediment plus water
         sediment.id = n;
         sediment.t  = e_grain*dx*a->w[i];

         if ( e_grain > 0 ) e_grain = c->remove( c->remove_data , a->x[i] , &sediment );
         if ( e_grain > 0 )
         {
            // Cubic meters of sediment
            e_grain *= (1-p);

//         if (a->h[i] < HMIN) a->c_grain[i][n]  = 0;
//         else                a->c_grain[i][n] += e_grain * dt / a->h[i];

            if ( a->h[i]>=HMIN ) a->c_grain[i][n] += e_grain / vol_w;

            eh_require( a->c_grain[i][n]>=0 );

            if ( a->c_grain[i][n]<0 ) a->c_grain[i][n] = 0.;
         }
         else
            e_grain = 0;

         ero        += e_grain;
         a->e[i][n] += e_grain;
      }

      eh_free( phe_bottom );
   }
   return ero;
}

double
sakura_deposit( Sakura_array* a , Sakura_sediment* sed , gint i , double dt , Sakura_const_st* c )
{
   double dep = 0;

   eh_require( a    );
   eh_require( sed  );
   eh_require( dt>0 );

   if ( a && sed && dt>0 )
   {
      if ( a->x[i] > c->dep_start )
      {
         gint           n;
         const gint     n_grains = sed->len;
         const double   dx       = a->x[i+1] - a->x[i];
         const double   vol_w    = dx*a->w[i]*a->h[i];
         Sakura_cell_st sediment;
         double         d_grain;
         double         p;
         double         small_h;
         double         avail;

         for ( n=0 ; n<n_grains ; n++ )
         {
            small_h = sed->u_settling[n] * dt * Ro;

            // Meters of sediment
            if ( a->h[i] <= small_h ) d_grain = a->h[i]/dt           *a->c_grain[i][n];
            else                      d_grain = sed->u_settling[n]*Ro*a->c_grain[i][n];

            p = ( sed->rho_grain[n] - sed->rho_dep[n] ) / ( sed->rho_grain[n] - c->rho_sea_water );

            // Meters of sediment plus water
            d_grain /= (1-p);

            // Cubic meters of sediment plus water
            sediment.id = n;
            sediment.t  = d_grain*dx*a->w[i]*dt;

            if ( d_grain > 0 ) d_grain = c->add( c->add_data , a->x[i] , &sediment );

            // Cubic meters of sediment
            d_grain *= (1-p);

//            if (a->h[i] < HMIN) a->c_grain[i][n]  = 0;
//            else                a->c_grain[i][n] -= d_grain * dt / (dx*a->w[i]*a->h[i]);

            avail = a->c_grain[i][n]*vol_w;

            if ( d_grain > avail ) d_grain = avail;

            a->c_grain[i][n] -= d_grain / vol_w;

            eh_require( a->c_grain[i][n]>=-1e-10 );

            if ( a->c_grain[i][n]<0 ) a->c_grain[i][n] = 0.;

            dep        += d_grain;
            a->d[i][n] += d_grain;
         }
      }
   }

   return dep;
}

gboolean
compute_c_grain_new( Sakura_array* a , Sakura_array* a_last , double* u , gint i , double dt , Sakura_const_st* c , Sakura_sediment* sed )
{
   gboolean success = TRUE;

   eh_require( a      );
   eh_require( a_last );
   eh_require( u      );
   eh_require( c      );
   eh_require( sed    );

   if ( dt>0 )
   {
      sakura_next_c_grain( a , a_last , u , i , dt , sed );
      sakura_erode       ( a , sed , u , i , dt , c );
      sakura_deposit     ( a , sed , i , dt , c );
   }
   return success;
}

// output c_new, sed_rate, CCMULTI_new
// input
gboolean
compute_c_grain( Sakura_array* a , Sakura_array* a_last , double* u , gint i , double dx , Sakura_const_st* Const , Sakura_sediment* sed )
{
   gboolean success = TRUE;

   eh_require( a        );
   eh_require( a_last   );
   eh_require( i>=0     );
   eh_require( i<a->len );

   if ( a && a_last )
   {
      gint   n;
      double sed_rate;
      double c_new;
      double h_0;
      double cll, cl, c_0, cr, crr;
      double ul, um, ur;
      double wl, wr;
      double hll, hl, hr, hrr;
      double small_h;
      double dh;
      double c_grain_new;
      double erode_depth;
      double depth_node;
      double porosity;
      double flux_at_bed;
      double rho_avg;
      double rho_bottom;
      double df_dt;
      const double    init_h         = a_last->h[-1];
      const double    dt             = Const->dt;
      const gint      n_grain        = a->n_grain;
      double*         phe_bottom     = eh_new( double , n_grain );
      double*         erosion        = eh_new( double , n_grain );
      Sakura_phe_func get_phe_func   = Const->get_phe;
      Sakura_add_func add_func       = Const->add;
      Sakura_add_func remove_func    = Const->remove;
      Sakura_get_func get_depth_func = Const->get_depth;
      Sakura_phe_st   phe_data;
      Sakura_cell_st  sediment;


      ul = u[i];
      ur = u[i+1];

      wl = a_last->w[i];
      wr = a_last->w[i+1]; 

      um = 0.5 * ( ul + ur );

      hll = a_last->h[i-2];
      hl  = a_last->h[i-1];
      h_0 = a_last->h[i  ];
      hr  = a_last->h[i+1];
      hrr = a_last->h[i+2];

      // compute CCMULTI for each grain size fraction
      for ( n = 0, sed_rate = 0, c_new = 0; n<n_grain ; n++)
      {

         small_h = sed->u_settling[n] * dt * Ro;

         cll = a_last->c_grain[i-2][n];
         cl  = a_last->c_grain[i-1][n];
         c_0 = a_last->c_grain[i  ][n];
         cr  = a_last->c_grain[i+1][n];
         crr = a_last->c_grain[i+2][n];

         eh_require( a->h[i]>=HMIN );
            
         if ( a->h[i]<HMIN )
         {
            c_grain_new = 0;
            //stopnumber = 1;
         }
         else
         {
            df_dt = dfdt(ul, ur, wl, wr, hl*cl, hr*cr, hll*cll, hrr*crr, h_0*c_0, dx, 0);
            c_grain_new = (c_0 * h_0 + dt * df_dt )/a->h[i];

            if ( c_grain_new < -HMIN)
            {
               eh_warning("negative new CC: node= %d, i= %d", i, n);
               eh_warning("cnew= %f, cold=%f", c_grain_new, c_0);
               c_grain_new = 0;
            }
         } //cnewi is the new concentration due to sediment transport by the flow

         // here we get the PheBottom at the node location.
//         erode_depth     = ( Const->c_drag * (1+c_grain_new*R)*Const->rho_sea_water * um*um - Const->sub );

         // amount of erosion in a time step
         erode_depth     = ( Const->c_drag * (1+c_grain_new*R)*Const->rho_sea_water * um*um - Const->sub )
                         / Const->sua * ( dt * S_DAYS_PER_SECOND );

//         phe_data.val      = erode_depth*dx;
         phe_data.val      = erode_depth*dx*a->w[i]; // Eroded volume
         phe_data.phe      = phe_bottom;
         phe_data.n_grains = a->n_grain;

         get_phe_func( Const->get_phe_data , a->x[i] , &phe_data );

         // get_phe_func may have changed the erosion depth if there wasn't enough sediment.
//         erode_depth = phe_data.val/dx;
         erode_depth = phe_data.val/(dx*a->w[i]);

         // Update the shear strength at the surface
         //a->sub[i] += Const->sua*erode_depth;

         rho_avg    = eh_dbl_array_mean_weighted( sed->rho_grain , a->n_grain , phe_bottom );
         rho_bottom = eh_dbl_array_mean_weighted( sed->rho_dep   , a->n_grain , phe_bottom );

         eh_require( rho_avg   >0 );
         eh_require( rho_bottom>0 );

         porosity = 1.0 - rho_bottom/rho_avg;

//         erosion[n] = erode_depth
//                    * phe_bottom[n]*(1.-porosity)
//                    / (Const->sua*S_SECONDS_PER_DAY);

         erosion[n] = erode_depth*phe_bottom[n]*(1.-porosity);

         if ( a->h[i] <= small_h) flux_at_bed = a->h[i]/dt              * c_grain_new - eh_max( 0 , erosion[n] ); 
         else                     flux_at_bed = sed->u_settling[n] * Ro * c_grain_new - eh_max( 0 , erosion[n] );

         if ( a->x[i] < Const->dep_start ) flux_at_bed = 0.0;

         depth_node = get_depth_func( Const->depth_data , a->x[i] );

         if ( depth_node + flux_at_bed*dt/porosity/phe_bottom[n] > -init_h )
         //if ( depth_node + flux_at_bed*dt/porosity/phe_bottom[n] > 0 )
            flux_at_bed = 0.0;

         dh = flux_at_bed*dt/porosity;

         // Keep track of volume eroded/deposited at each node and for each grain size
         sediment.t  = dh*dx*a->w[i];
         sediment.id = n;
         if ( dh<0 ) a->e[i][n] += remove_func( Const->remove_data , a->x[i] , &sediment );
         else        a->d[i][n] += add_func   ( Const->add_data    , a->x[i] , &sediment );

//         SEDMULTI[i][n] += flux_at_bed * dt / porosity;

         if (a->h[i] < HMIN) a->c_grain[i][n] = 0;
         else                a->c_grain[i][n] = c_grain_new - flux_at_bed * dt/ a->h[i];

         eh_require( a->c_grain[i][n] >= -HMIN );

         if (a->c_grain[i][n] < -HMIN)
         {
            eh_warning("negative CCMULTInew: node=%d, grain=%d",i,n);
            eh_warning("cnew=%f, cold=%f, hnew=%f", a->c_grain[i][n], c_grain_new , a->h[i]);
         }
         else if ( a->c_grain[i][n] < 0)
            a->c_grain[i][n] = 0.0;

         a->c_grain[i][n] = eh_max( 0 , a->c_grain[i][n] );

         c_new    += a->c_grain[i][n];
         sed_rate += flux_at_bed;
      } //end of CCMULTI

      a->c[i]     = c_new;
//      a->s[i]    += dt*sed_rate;
//      a->r[i]    += sed_rate;

      eh_free( phe_bottom );
      eh_free( erosion    );
/*
      max_c       = eh_max( max_c , c_new );
      total_susp += c_new * a->h[i];

      if ( max_c<=HMIN || total_susp<=HMIN )
      {
         eh_warning("maxc=%f, totalsusp=%f", max_c, total_susp);
         eh_warning("ccmultinew=%f, hew=%f",a->c_grain[i-1][n-1], a->h[i-1]);
         eh_watch_int( node );
         eh_watch_dbl( HMIN );
         stopnumber = 1;
      }
*/
      
   }
   else
      success = FALSE;

   return success;
}

gboolean
compute_next_h( Sakura_array* a_new , Sakura_array* a_last , double* u_temp , gint ind_head , Sakura_const_st* c )
{
   gboolean success = TRUE;

   if ( a_new && a_last )
   {
      const double dt    = c->dt;
      const gint   top_i = a_new->len-2;
      const double dx    = a_new->x[1] - a_new->x[0];
      gint i;
      double hll, hl, h_0, hr, hrr;
      double ul, um, ur;
      double wl, wr;
      double c_0;
      double Ew, Ri;
      double df_dt;

      ind_head = eh_min( ind_head , a_last->len-1 );

      // START STEP2: calculate new flow thickness and sediment concentration
      // calculations at node midpoint for HH, CC and SED
      //  uses Utemp to get HHnew and CCnew
      for ( i=0 ; i<=ind_head && i<=top_i ; i++ )
      {
         h_0 = a_last->h[i];
         c_0 = a_last->c[i];
         
         ul = u_temp[i];
         ur = u_temp[i+1];

         wl = a_last->w[i];
         wr = a_last->w[i+1]; 

         um = 0.5 * ( ul + ur );

         hll = a_last->h[i-2];
         hl  = a_last->h[i-1];
         hr  = a_last->h[i+1];
         hrr = a_last->h[i+2];
         
         // compute water entrainment 
         if ( eh_compare_dbl( um , 0.0 , 1e-12 ) )
         {
            Ew = 0.0;
            Ri = 0.0;
         }
         else
         //if (um != 0.0)
         {
            Ri = R * G * c_0 * h_0 / eh_sqr(um);
            Ew = c->e_a / (c->e_b + Ri);
         }

         df_dt = dfdt(ul, ur, wl, wr, hl, hr, hll, hrr, h_0, dx, Ew*fabs(um));
         
         // compute new HH
         a_new->h[i] = h_0 + dt * df_dt;

         eh_require( a_new->h[i]>=0 );

         if (a_new->h[i] < 0)
         {
            eh_warning( "HHnew negative but cancelled at the %d-th node" ,i);
            eh_warning( "ul:%f, ur:%f, hl:%f, h:%f, hr:%f", ul,ur,hl,h_0,hr);
            a_new->h[i] = 0;
         }
      }
   }

   return success;
}

gboolean
compute_next_c( Sakura_array* a_new , Sakura_array* a_last , double* u_temp , gint ind_head , Sakura_const_st* c , Sakura_sediment* sed )
{
   gboolean success = TRUE;

   if ( a_new && a_last )
   {
      gint         i;
      const gint   top_i = a_new->len-2;
      const double dt    = c->dt;

      ind_head = eh_min( ind_head , a_last->len-1 );

      for ( i=0 ; i<=ind_head && i<=top_i ; i++ )
      {
         sakura_next_c_grain( a_new , a_last , u_temp , i , dt , sed );
         sakura_erode       ( a_new , sed , u_temp  , i , dt , c );
         sakura_deposit     ( a_new , sed , i , dt , c );
      }
   }

   return success;
}

// output HH_new, CC_new, and SED_new at t+dt
// input HH, CC, Wx
gboolean
calculate_next_c_and_h( Sakura_array* a_new , Sakura_array* a_last , double* u_temp , gint ind_head , Sakura_const_st* Const , Sakura_sediment* sed )
{
   gboolean success = TRUE;

   if ( a_new && a_last )
   {
      const double dt    = Const->dt;
      const gint   top_i = a_new->len-2;
      const double dx    = a_new->x[1] - a_new->x[0];
      gint i;
      double hll, hl, h_0, hr, hrr;
      double ul, um, ur;
      double wl, wr;
      double c_0;
      double Ew, Ri;
      double df_dt;

      // START STEP2: calculate new flow thickness and sediment concentration
      // calculations at node midpoint for HH, CC and SED
      //  uses Utemp to get HHnew and CCnew
      for ( i=0 ; i<=ind_head && i<=top_i ; i++ )
      {
         h_0 = a_last->h[i];
         c_0 = a_last->c[i];
         
         ul = u_temp[i];
         ur = u_temp[i+1];

         wl = a_last->w[i];
         wr = a_last->w[i+1]; 

         um = 0.5 * ( ul + ur );

         hll = a_last->h[i-2];
         hl  = a_last->h[i-1];
         hr  = a_last->h[i+1];
         hrr = a_last->h[i+2];
         
         // compute water entrainment 
         if ( eh_compare_dbl( um , 0.0 , 1e-12 ) )
         {
            Ew = 0.0;
            Ri = 0.0;
         }
         else
         //if (um != 0.0)
         {
            Ri = R * G * c_0 * h_0 / eh_sqr(um);
            Ew = Const->e_a / (Const->e_b + Ri);
         }

         df_dt = dfdt(ul, ur, wl, wr, hl, hr, hll, hrr, h_0, dx, Ew*fabs(um));
         
         // compute new HH
         a_new->h[i] = h_0 + dt * df_dt;

         eh_require( a_new->h[i]>=0 );

         if (a_new->h[i] < 0)
         {
            eh_warning( "HHnew negative but cancelled at the %d-th node" ,i);
/*
            eh_warning( "old=%f, new=%f, dfdt=%f, left=%f, right=%f"      ,h,HHnew[node],dfdt(ul,ur,wl,wr,hl,hr,hll,hrr,h,Dx,Ew*um),tvdleft(ul,h,hl,hr,hll,hrr),tvdright(ur,h,hl,hr,hll,hrr));
*/
            eh_warning( "ul:%f, ur:%f, hl:%f, h:%f, hr:%f", ul,ur,hl,h_0,hr);
            a_new->h[i] = 0;
         }

//                     if (HHnew[node] > LARGER(InitH, fabs(DEPTH[node])) )
 //                        HHnew[node] = -DEPTH[node];

         // compute CCMULTI for each grain size fraction
         compute_c_grain( a_new , a_last , u_temp , i , dx , Const , sed );

         //a->c[i]     = c_new;
         //a->s[i]    += dt*sed_rate;
         //a->r[i]    += sed_rate;

         //max_c       = eh_max( max_c , c_new );
         //total_susp += c_new * a->h[i];
      } /*end STEP2*/
   }
   else
      success = FALSE;

   return success;
}

gboolean
calculate_mid_c_and_h( Sakura_array* a_mid , Sakura_array* a_last , Sakura_array* a_next )
{
   gboolean success = TRUE;

   if ( a_mid && a_last && a_next )
   {
      gint i;
      gint top_i = a_mid->len+2;

      for ( i=-2 ; i<top_i ; i++ )
      {
         a_mid->c[i] = .5*( a_last->c[i] + a_next->c[i] );
         a_mid->h[i] = .5*( a_last->h[i] + a_next->h[i] );
      }
   }
   return success;
}

gint
calculate_head_index( Sakura_array* a , double* u , gint ind_head , double dx , double dt , double* x_head )
{
   gint new_ind = -1;

   eh_require( a );
   eh_require( u );

   if ( a && u )
   {
      eh_require( ind_head>=0     );

      if ( ind_head<a->len )
      {
         double u_head = eh_max( u[ind_head] , u[ind_head-1] );

         if      ( ind_head<=0 ) u_head = u[0];
         else if ( u_head   >0 ) u_head = eh_min( u_head , 1.5*pow( G*R*a->c[ind_head-1]*a->h[ind_head-1] ,1./3.) );
         else                    u_head = eh_max( u[ind_head] , u[ind_head-1] );

         *x_head += u_head * dt;
         new_ind  = floor( (*x_head-a->x[0]) / dx );
      }
      else
         new_ind = ind_head;

      eh_require( new_ind>=0     );
   }

   return new_ind;
}

/** Run the sakura hyperpycnal flow model

\param dx        [UNUSED] Grid spacing (m)
\param dt        Time step (s)
\param basin_len [UNUSED]
\param n_nodes   Number of grid nodes
\param n_grains  Number of sediment types
\param Xx        Positions of grid nodes (m)
\param Zz        Elevations of grid nodes (m)
\param Wx        Channel width at grid nodes (m)
\param u_init    River velocity (m/s)
\param c_init    River volume concentration (m^3/m^3)
\param lambda    Removal rates for sediment types [UNUSED]
\param u_settling Settling velocitiesfor sediment types (m/s)
\param Rey        [UNUSED]
\param rho_grain Grain density of sediment types (kg/m^3)
\param h_init    River depth
\param supply_time Duration of flow (s)
\param DepositionStart [UNUSED]
\param fraction  Fraction of each grain type in river
\param bottom_f  [UNUSED]
\param rho_dep   Bulk density of each grain type when deposited on the sea floor (kg/m^3)
\param OutTime   [UNUSED]
\param c         Structure of constants used in model
\param Deposit   [UNUSED]
\param fp_data   [UNUSED]

\return TRUE on success of FALSE if an error occured
*/
/*
gboolean
sakura( double dx          , double dt              , double basin_len ,
        int n_nodes        , int n_grains           , double Xx[]      ,
        double Zz[]        , double Wx[]            , double u_init[]  ,
        double c_init[]    , double *Lambda         , double* u_settling ,
        double *Rey        , double *rho_grain      , double h_init    ,
        double supply_time , double DepositionStart , double *fraction ,
        double *bottom_f   , double* rho_dep        , double OutTime   ,
        Sakura_const_st* c , double **Deposit       , FILE *fp_data )
*/
/** Run the sakura hyperpycnal flow model

\param u_riv     River velocity (m/s)
\param c_riv     River concentration (kg/m^3)
\param h_riv     River depth (m)
\param f_riv     Fraction of each grain type in river
\param dt        Time step to use (s)
\param duration  Duration of flow (s)
\param x         Position of grid nodes (m)
\param z         Elevation of grid nodes (m)
\param w         Channel widht at grid nodes (m)
\param n_nodes   Number of grid nodes
\param rho_grain Grain density of sediment types (kg/m^3)
\param rho_dep   Bulk density of each grain type when deposited on the sea floor (kg/m^3)
\param u_fall    Settling velocitiesfor sediment types (m/s)
\param n_grains  Number of sediment types
\param c         Structure of constants used in model

\return TRUE on success of FALSE if an error occured
*/
gboolean
sakura( double  u_riv     , double  c_riv    , double  h_riv  , double* f_riv    ,
        double  dt        , double  duration ,
        double* x         , double* z        , double* w      , gint    n_nodes  ,
        double* rho_grain , double* rho_dep  , double* u_fall , gint    n_grains ,
        Sakura_const_st* c )
{
   gboolean success = TRUE;

   eh_require( u_riv>0    );
   eh_require( c_riv>0    );
   eh_require( h_riv>0    );
   eh_require( f_riv      );
   eh_require( dt>0       );
   eh_require( duration>0 );
   eh_require( x          );
   eh_require( z          );
   eh_require( w          );
   eh_require( n_nodes>0  );
   eh_require( rho_grain  );
   eh_require( rho_dep    );
   eh_require( u_fall     );
   eh_require( n_grains>0 );
   eh_require( c          );

   if ( dt>0 )
   { // Run the model for positive time steps
      Sakura_array*    a_next  = sakura_array_new( n_nodes , n_grains );
      Sakura_array*    a_mid   = sakura_array_new( n_nodes , n_grains );
      Sakura_array*    a_last  = sakura_array_new( n_nodes , n_grains );
      Sakura_node*     inflow  = NULL;
      Sakura_node*     outflow = NULL;
      Sakura_sediment* sed     = sakura_sediment_new( n_grains );

      { // Set the inflow and outflow conditions
         gint    n;
         double* c_grain = eh_dbl_array_dup( f_riv , n_grains );

         for ( n=0 ; n<n_grains ; n++ )
            c_grain[n] = (c_riv*f_riv[n])/rho_grain[n];

         // Convert river concentration to volume concentration
         c_riv = eh_dbl_array_sum( c_grain , n_grains );

         inflow  = sakura_node_new( u_riv , c_riv , h_riv , c_grain , n_grains );

         c_grain = eh_dbl_array_set( c_grain , n_grains , 0. );
         outflow = sakura_node_new( 0. , 0. , 0. , c_grain , n_grains );

         eh_free( c_grain );
      }

      { // Initialize arrays
         sakura_array_set_x( a_next , x );
         sakura_array_set_x( a_mid  , x );
         sakura_array_set_x( a_last , x );
         sakura_array_set_w( a_next , w );
         sakura_array_set_w( a_mid  , w );
         sakura_array_set_w( a_last , w );
      }

      { // Initialize sediment
         sakura_sediment_set_rho_dep   ( sed , rho_dep   );
         sakura_sediment_set_rho_grain ( sed , rho_grain );
         sakura_sediment_set_u_settling( sed , u_fall    );
      }

      c->sua       *= 1e3;
      c->sub       *= 1e3;
      c->dep_start += x[0];

      if ( g_getenv( "SAKURA_DEBUG" ) )
      { // Print input variables for debugging
         gint i, n;

         eh_debug( "Supply time        : %f" , duration     );
         eh_debug( "Init velocity      : %f" , inflow->u    );
         eh_debug( "Init concentration : %f" , inflow->c    );
         eh_debug( "Init height        : %f" , inflow->h    );
         eh_debug( "Time step          : %f" , dt           );
         eh_debug( "Number of nodes    : %d" , n_nodes      );
         eh_debug( "Number of grains   : %d" , n_grains     );
   
         for ( n=0 ; n<n_grains ; n++ )
         {
            eh_debug( "Grain Type: %d" , n );
            eh_debug( "   Settling velocity (m/d)  : %f" , u_fall[n]*S_SECONDS_PER_DAY );
            eh_debug( "   Grain density (kg/m^3)   : %f" , rho_grain[n] );
            eh_debug( "   Deposit density (kg/m^3) : %f" , rho_dep[n] );
            eh_debug( "   Fraction                 : %f" , f_riv[n] );
         }
   
         eh_debug( "c->dt  : %f" , c->dt  );
         eh_debug( "c->sua : %f" , c->sua );
         eh_debug( "c->sub : %f" , c->sub );
         eh_debug( "c->e_a : %f" , c->e_a );
         eh_debug( "c->e_b : %f" , c->e_b );
         eh_debug( "c->c_drag : %f" , c->c_drag );
         eh_debug( "c->mu_water : %f" , c->mu_water );
         eh_debug( "c->rho_sea_water : %f" , c->rho_sea_water );
         eh_debug( "c->dep_start : %f" , c->dep_start );
      }
   
      { // Run the model
         const double dx       = a_last->x[1] - a_last->x[0];
         double       x_head   = HMIN + a_last->x[0];
         gint         ind_head = floor( (x_head-a_last->x[0])/dx );
         double       t;
   
         eh_require( x_head>0         );
         eh_require( ind_head>=0      );
         eh_require( ind_head<n_nodes );
   
         for ( t=0. ; t<=duration && success ; t+=dt )
         { // Run the flow for each time step
   
      eh_watch_dbl( t );

      if ( t>duration )
      {
         inflow->u = 0;
         inflow->c = 0;
         inflow->h = 0;
      }
   
            sakura_set_outflow ( outflow , a_last , x_head  , dt , dx );
            sakura_array_set_bc( a_last  , inflow , outflow );
            sakura_array_set_bc( a_mid   , inflow , outflow );
   
            // Calculate u at t+dt/2.  The rest of a_mid is invalid.  a_mid->u is u_temp.
            // This also calculates u at the head of the flow
            calculate_mid_vel( a_mid  , a_last , ind_head , c );
   
            // Calculate c, and h at t+dt.  This is a_next.  a_next->u is not valid.
            compute_next_h( a_next , a_last , a_mid->u , ind_head , c );
            compute_next_c( a_next , a_last , a_mid->u , ind_head , c , sed );
   
            // Set new boundary conditions
            sakura_set_outflow ( outflow , a_last , x_head , dt , dx );
            sakura_array_set_bc( a_next  , inflow , outflow );
   
            // Calculate c, and h at t+dt/2.  This is in a_mid and an average of a_last and a_next
            calculate_mid_c_and_h( a_mid , a_last , a_next );
   
            // Calculate u at t+dt.  This is a_next->u.
            calculate_next_vel( a_last , a_mid , a_next , ind_head , c );
   
            ind_head = calculate_head_index( a_last , a_mid->u , ind_head , dx , dt , &x_head );
   
            // Update variables
            sakura_array_copy( a_last , a_next );
         }
      }

      if ( TRUE || g_getenv( "SAKURA_DEBUG" ) )
      { // Mass balance check
         gint         n;
         double       mass_in        = 0;
         double       mass_out       = 0;
         const double mass_in_susp   = sakura_array_mass_in_susp  ( a_last , sed );
         const double mass_eroded    = sakura_array_mass_eroded   ( a_last , sed );
         const double mass_deposited = sakura_array_mass_deposited( a_last , sed );
         const double vol_w          = u_riv*h_riv*w[0]*duration;

         // c_riv is now volume concentration
         for ( n=0,mass_in=0 ; n<n_grains ; n++ )
            mass_in += c_riv*f_riv[n]*vol_w*rho_grain[n];

         eh_watch_dbl( mass_in  );
         eh_watch_dbl( mass_in_susp );
         eh_watch_dbl( mass_eroded );
         eh_watch_dbl( mass_deposited );

         mass_in  += sakura_array_mass_eroded( a_last , sed );
         mass_out  = sakura_array_mass_in_susp  ( a_last , sed )
                   + sakura_array_mass_deposited( a_last , sed );

         if ( !eh_compare_dbl(mass_in,mass_out,.01) )
            eh_warning( "Mass balance check failed" );

         eh_watch_dbl( mass_in  );
         eh_watch_dbl( mass_out );

      }

      { // de-allocate memory
         sakura_array_destroy( a_next );
         sakura_array_destroy( a_mid  );
         sakura_array_destroy( a_last );
   
         sakura_sediment_destroy( sed );
   
         sakura_node_destroy( inflow  );
         sakura_node_destroy( outflow );
      }
   }

   return success;
}

#endif

