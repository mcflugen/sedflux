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

#if !defined( GLIB_H )
#define GLIB_H

#include "glib.h"
#include <utils/utils.h>

double *solve_excess_pore_pressure( double *psi , double *k , double *c , int n , double dz , double dt , double psi_top , double sed_rate );
void get_matrix_coefficients( double *psi , double *k , double *c , double ds , double dz , double dt , double psi_top , int n , double f ,  double *l , double *d , double *u , double *b );

double *solve_excess_pore_pressure_mg( double *psi , double *k , double *c , int n , double dz , double dt , double sed_rate );
double *restrict_1d( double *u_2h , double *u_h , int n_h );
double *inter_1d( double *u_h , double *u_2h , int n_2h );
double *relax_1d( double *u , double *k , double *f , int n , double dz , double dt );
double *residual_1d( double *res , double *u , double *k , double *f , int n , double dz , double dt );
double *solve_1d( double *u , double *k , double *f , double dz , double dt );
double *add_inter_1d( double *u_h , double *u_2h , int n_2h );
double *mgm_1d( double *u_h , double *k_h , double *f_h , int n_h , double dz , double dt );
double *fmg_1d( double *u_h , double *k_h , double *f_h , int n_h , double dz , double dt );

double **solve_excess_pore_pressure_mg_2d( double **psi , double **kx , double **kz , double **c , int n , double dx , double dz , double dt , double *sed_rate );
double **restrict_2d( double **u_2h , double **u_h , int n_h );
double **inter_2d( double **u_h , double **u_2h , int n_2h );
double **relax_2d( double **u , double **kx , double **kz , double **f , int n , double dx , double dz , double dt );
double **residual_2d( double **res , double **u , double **kx , double **kz , double **f , int n , double dx , double dz , double dt );
double **solve_2d( double **u , double **kx , double **kz , double **f , double dx , double dz , double dt );
double **add_inter_2d( double **u_h , double **u_2h , int n_2h );
double **mgm_2d( double **u_h , double **kx_h , double **kz_h , double **f_h , int n_h , double dx , double dz , double dt );
double **fmg_2d( double **u_h , double **kx_h , double **kz_h , double **f_h , int n_h , double dx , double dz , double dt );

double ***solve_excess_pore_pressure_mg_3d( double ***psi , double ***kx , double ***kz , double ***c , int n , double dx , double dz , double dt , double **sed_rate );
double ***restrict_3d( double ***u_2h , double ***u_h , int n_h );
double ***inter_3d( double ***u_h , double ***u_2h , int n_2h );
double ***relax_3d( double ***u , double ***kx , double ***kz , double ***f , int n , double dx , double dz , double dt );
double ***residual_3d( double ***res , double ***u , double ***kx , double ***kz , double ***f , int n , double dx , double dz , double dt );
double ***solve_3d( double ***u , double ***kx , double ***kz , double ***f , double dx , double dz , double dt );
double ***add_inter_3d( double ***u_h , double ***u_2h , int n_2h );
double ***mgm_3d( double ***u_h , double ***kx_h , double ***kz_h , double ***f_h , int n_h , double dx , double dz , double dt );
double ***fmg_3d( double ***u_h , double ***kx_h , double ***kz_h , double ***f_h , int n_h , double dx , double dz , double dt );

double *allocate_1d( int );
double **allocate_2d( int );
double ***allocate_3d( int );
void free_2d( double** );
void free_3d( double*** );

#endif

