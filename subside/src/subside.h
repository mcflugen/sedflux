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

/** \file subside.h

   Flexural subsidence header file.

   \defgroup subsidence_group Flexural Subsidence

   \callgraph
*/
/*@{*/

#ifndef _ISOSTASY_H_
# define _ISOSTASY_H_

# include "utils.h"
# include "sed_sedflux.h"

/**
   \brief Solve the flexure equation for a point load.

   \f[
      w(x) = {q \lambda \over 2 \rho_m g}
             e^{ -\lambda x } \left( \cos (\lambda x) + \sin (\lambda x) \right)
   \f]

   \param g      The grid to subside
   \param load   The load
   \param h      The effective elastic thickness
   \param E      Young's modulus
   \param i_load The x-index of the point load
   \param j_load The y-index of the point load
*/
void subside_point_load      ( Eh_dbl_grid g , double load , double h ,
                               double E      , int i_load  , int j_load );
/**
   \brief Solve the flexure equation for a half-plane load.

   The solution for a half-plane load is,

   \f[
      w(r) = {q \over \rho_m g } e^{ -\lambda r } \cos (\lambda r)
   \f]

   where \f$ r \f$ is the distance from the edge of the load.

   The load is assumed to begin on the right-hand edge of the grid and
   continue to infinity.

   \note Currently this function only operates on 1D grids.  If a 2D grid
   is used, this function silently does nothing.

   \param g The grid to subside
   \param load The load
   \param h The effective elastic thickness
   \param E Young's modulus

   \todo This needs to be extended to 2D.
*/
void subside_half_plane_load ( Eh_dbl_grid g ,
                               double load   ,
                               double h      ,
                               double E );

/**
   \brief Calculate the flexure parameter

   The flexure parameter is defined as,

   \f[
      \lambda \equiv \cases{ \root 4 \of { \rho_m g \over 4 D },&$n=1$\cr
                             \root 4 \of { \rho_m g \over   D },&$n=2$\cr}
   \f]

   @param h The effective elastic thickness
   @param E Young's modulus
   @param n_dim The number of spatial dimensions (1 or 2)

*/
double get_flexure_parameter ( double h       , double E    , gssize n_dim );

#endif

/*@}*/
