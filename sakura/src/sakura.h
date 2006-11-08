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

#if !defined(SAKURA_H)
#define SAKURA_H

#include <glib.h>

// LIST OF FUNCTIONS defined only for sakura
// define constants
#define HMIN 0.0000001
#define UPPERLIMIT 20

//
// define general constants
//

/** @name Physical Constants */
//@{
/// gravitational acceleration (m/s/s)
#define G (9.81)

/// submerged specific gravity of quartz
#define R (1.65)

/// near-bed bulk concentration ratio
#define Ro (2.0)

/// Length of a day in seconds.
# define DAY (86400.)
//@}

/** @name Convenience Macros */
//@{
/// The square of a value.
#define SQ(A) (A * A)
/// Sign of a value.
#define	SIGN(A) ((A) > (0.0) ? (1.0) : (-1.0))
/// The larger of two values.
#define LARGER(A,B) ((A)>(B) ? (A):(B))
/// The smaller of two values.
#define SMALLER(A,B) ((A)<(B) ? (A):(B))
//@}

/**
Specifies the type of functions that are used to query the sedflux
architecture.

@param query_data A pointer to the data that specifies the user's query.  This
                  data may contain things like a location, depth, or
                  sediment property.  Any output data should also be contained
                  within this data structure.
@param data       A pointer to data from the architecture that is to be
                  queried.

@return Nothing.
*/
typedef void (*Eh_query_func) ( gpointer query_data , gpointer data );

/** @memo Sakura_t

@doc The Sakura_t structure holds physical constants that are required for the
turbidity current model.  Also included are pointers to various Eh_query_func's
that can be used to query the architecture.  Here there are functions to add
sediment, remove sediment, and get the grain size distribution.

*/
typedef struct
{
/// Entrainment coefficient.
   double Ea;
/// Entrainment coefficient.
   double Eb;
   double sua;
   double sub;
/// Drag coefficient.
   double Cd;
   double tanPhi;
   double mu;
/// Density of sea water.
   double rhoSW;
/// Density of river water.
   double rhoRW;
/// Width of the channel.
   double channelWidth;
/// Length of the channel.
   double channelLength;

/// Function that gets the grain size distribution.
   Eh_query_func get_phe;
/// Function that adds sediment to a location.
   Eh_query_func add;
/// Function that removes sediment from a location.
   Eh_query_func remove;
/// Function that gets the depth at an index.
   Eh_query_func get_depth;
/// Function that sets the depth at an index.
   Eh_query_func set_depth;
/// Architecture data used for the get_phe function.
   gpointer get_phe_data;
/// Architecture data used for the add function.
   gpointer add_data;
/// Architecture data used for the remove function.
   gpointer remove_data;
/// Architecture data used for the set/get depth functions.
   gpointer depth_data;
}
Sakura_t;

/** @memo Sak_get_phe_t

@doc The Sak_get_phe_t structure describes the grain size distribution of
the bottom sediments used in the sakura turbidity current model. It is intended
to be passed as the data parameter of a Eh_query_func.

The phe_bottom member gives the fraction of each grain type that is found on
the sea floor.

The n_grains member is the number of grain types (the length of phe_bottom ).

@see Eh_query_func , sakura_get_phe .

*/
typedef struct
{
/// The grain type distribution of bottom sediments.
   double *phe_bottom;
/// The number of grain types.
   int n_grains;
}
Sak_bottom_t;

/** @memo Sak_bathy_t

@doc The Sak_bathy_t structure describes the bathymetric architecture for the
sakura model.  With this information a user is able to query bathymetric data
(such as slope or depth) or add and remove sediment from a location.

*/
typedef struct
{
/// Horizontal position.
   double *x;
/// Bathymetric elevation (a larger negative number is deeper).
   double *depth;
/// The bathymetric slope at a location.
   double *slope;
/// The number of nodes in the model.  This is the length of the various
/// arrays.
   int n_nodes;
}
Sak_bathy_t;

/** @memo Sak_depth_query_t

@doc This structure provides the information that is required to get/set the
depth of the bathymetry of the sakura architecture.
*/
typedef struct
{
   int i;
   double depth;
}
Sak_depth_query_t;

/** @memo Sak_erode_query_t

@doc This structure provides the information that is required to erode sediment
from the sakura architecture.
*/
typedef struct
{
/// The amount of sediment to erode.
   double dh;
/// The index of the node to erode.
   int i;
}
Sak_erode_query_t;

/** @memo Sak_add_query_t

@doc This structure provides the information that is required to add sediment
to the sakura architecture.

@see Sak_remove_query_t .
*/
typedef Sak_erode_query_t Sak_add_query_t;

/** @memo Sak_phe_query_t

@doc The Sak_phe_query_t structure describes the grain size distribution of
the bottom sediments used in the sakura turbidity current model. It is intended
to be passed as the data parameter of a Eh_query_func that communicates with
the sedflux architecture.

The dx member specifies the column spacing in the Sed_cube.

The x member gives the position of the sediment that is to be queried.

The erode_depth is the depth of erosion.  All of the sediment up to this depth
is averaged to find the grain size distibution.  This value should be changed
if there is not enough sediment at the particular location.

The phe member points to the location to write the grain size distribution to.

@see Eh_query_func , sakura_get_phe .

*/
typedef struct
{
/// The column spacing.
   double dx;
/// The position of the sediment where the query will be made.
   double x;
/// The depth of erosion.
   double erode_depth;
/// Array to hold the grain type distribution.
   double *phe;
}
Sak_phe_query_t;

int sakura ( double Dx         , double Dt              , double Basin_Len ,
             int nNodes        , int nGrains            , double Xx[]      ,
             double Zz[]       , double Wx[]            , double Init_U[]  ,
             double Init_C[]   , double *Lambda         , double *Stv      ,
             double *Rey       , double *gDens          , double InitH     ,
             double SupplyTime , double DepositionStart , double *Fraction ,
             double *PheBottom , double *DepositDensity , double OutTime   ,
             Sakura_t Consts   , double **Deposit       , FILE *fp_data );
/*
int sakura( double Dx         , double Dt              , double Basin_Len ,
            int nNodes        , int nGrains            , double Xx[]      ,
            double Slope[]    , double *Lambda         , double *Stv      ,
            double *Rey       , double *gDens          , double InitH     ,
            double InitU      , double InitC           , double *Fraction ,
            double *PheBottom , double *DepositDensity , double OutTime   ,
            double MaxTime    , Sakura_t Consts        , double **Deposit ,
            FILE *Outfp2 );
*/

#endif /* sakura.h is included */


