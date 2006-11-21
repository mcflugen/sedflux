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

#if !defined(__SAKURA_H__)
#define __SAKURA_H__

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

/** Define physica constants for the sakura module

The Sakura_t structure holds physical constants that are required for the
turbidity current model.  Also included are pointers to various Eh_query_func's
that can be used to query the architecture.  Here there are functions to add
sediment, remove sediment, and get the grain size distribution.

*/
typedef struct
{
   double Ea;            ///< Entrainment coefficient.
   double Eb;            ///< Entrainment coefficient.
   double sua;           ///< Shear strength at the seafloor
   double sub;           ///< Rate of change of shear strength with burial depth
   double Cd;            ///< Drag coefficient.
   double tanPhi;
   double mu;
   double rhoSW;         ///< Density of sea water.
   double rhoRW;         ///< Density of river water.
   double channelWidth;  ///< Width of the channel.
   double channelLength; ///< Length of the channel.

   Eh_query_func get_phe;   ///< Function that gets the grain size distribution.
   Eh_query_func add;       ///< Function that adds sediment to a location.
   Eh_query_func remove;    ///< Function that removes sediment from a location.
   Eh_query_func get_depth; ///< Function that gets the depth at an index.
   Eh_query_func set_depth; ///< Function that sets the depth at an index.
   gpointer get_phe_data;   ///< Architecture data used for the get_phe function.
   gpointer add_data;       ///< Architecture data used for the add function.
   gpointer remove_data;    ///< Architecture data used for the remove function.
   gpointer depth_data;     ///< Architecture data used for the set/get depth functions.
}
Sakura_t;

/**  Defines grain-size information for the sakura module

The Sak_get_phe_t structure describes the grain size distribution of
the bottom sediments used in the sakura turbidity current model. It is intended
to be passed as the data parameter of a Eh_query_func.

The phe_bottom member gives the fraction of each grain type that is found on
the sea floor.

The n_grains member is the number of grain types (the length of phe_bottom ).

@see Eh_query_func , sakura_get_phe .

*/
typedef struct
{
   double *phe_bottom; ///< The grain type distribution of bottom sediments.
   int n_grains;       ///< The number of grain types.
}
Sak_bottom_t;

/** Defines bathymetry within the sakura module

The Sak_bathy_t structure describes the bathymetric architecture for the
sakura model.  With this information a user is able to query bathymetric data
(such as slope or depth) or add and remove sediment from a location.

*/
typedef struct
{
   double *x;     ///< Horizontal position
   double *depth; ///< Bathymetric elevation (a larger negative number is deeper).
   double *slope; ///< The bathymetric slope at a location.
   int n_nodes;   ///< The number of nodes in the model.  This is the length of the various arrays.
}
Sak_bathy_t;

/** Holds data for depth queries for sakura

This structure provides the information that is required to get/set the
depth of the bathymetry of the sakura architecture.
*/
typedef struct
{
   int i;
   double depth;
}
Sak_depth_query_t;

/** Holds data for erosion queries for sakura

This structure provides the information that is required to erode sediment
from the sakura architecture.
*/
typedef struct
{
   double dh; ///< The amount of sediment to erode.
   int i;     ///< The index of the node to erode.
}
Sak_erode_query_t;

/** Holds data for deposition queries for sakura

This structure provides the information that is required to add sediment
to the sakura architecture.

@see Sak_remove_query_t .
*/
typedef Sak_erode_query_t Sak_add_query_t;

/** Holds data for grain-size queries for sakura

The Sak_phe_query_t structure describes the grain size distribution of
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
   double dx;          ///< The column spacing.
   double x;           ///< The position of the sediment where the query will be made.
   double erode_depth; ///< The depth of erosion.
   double *phe;        ///< Array to hold the grain type distribution.
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

#endif /* sakura.h is included */


